#include "collision.h"
#include "toy_hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define TABLE_SIZE (1ULL << 26)
#define NUM_PARTITIONS 128
#define PARTITION_BITS 7
#define PARTITION_SIZE (TABLE_SIZE / NUM_PARTITIONS)
#define PARTITION_MASK (NUM_PARTITIONS - 1)
#define PARTITION_SIZE_MASK (PARTITION_SIZE - 1)

typedef struct
{
    HashEntry *entries;
    omp_lock_t locks[NUM_PARTITIONS];
} PartitionedHashTable;

static int table_init(PartitionedHashTable *table)
{
    table->entries = calloc(TABLE_SIZE, sizeof(HashEntry));
    if (table->entries == NULL)
        return 0;

    for (int i = 0; i < NUM_PARTITIONS; i++)
        omp_init_lock(&table->locks[i]);

    return 1;
}

static void table_destroy(PartitionedHashTable *table)
{
    for (int i = 0; i < NUM_PARTITIONS; i++)
        omp_destroy_lock(&table->locks[i]);
    free(table->entries);
}

static inline int table_partition(uint64_t hash)
{
    return (int)(hash & PARTITION_MASK);
}

static inline size_t table_start_index(uint64_t hash)
{
    return (size_t)((hash >> PARTITION_BITS) & PARTITION_SIZE_MASK);
}

static int table_insert(PartitionedHashTable *table, uint64_t hash, uint64_t nonce)
{
    int partition = table_partition(hash);
    size_t offset = (size_t)partition * PARTITION_SIZE;
    size_t index = table_start_index(hash);

    omp_set_lock(&table->locks[partition]);

    size_t probes = 0;
    while (table->entries[offset + index].occupied)
    {
        index = (index + 1) & PARTITION_SIZE_MASK;
        probes++;
        if (probes == PARTITION_SIZE)
        {
            omp_unset_lock(&table->locks[partition]);
            return 0;
        }
    }

    table->entries[offset + index].hash = hash;
    table->entries[offset + index].nonce = nonce;
    table->entries[offset + index].occupied = 1;

    omp_unset_lock(&table->locks[partition]);
    return 1;
}

static int table_lookup(const PartitionedHashTable *table, uint64_t hash, uint64_t *out_nonce)
{
    int partition = table_partition(hash);
    size_t offset = (size_t)partition * PARTITION_SIZE;
    size_t index = table_start_index(hash);

    while (table->entries[offset + index].occupied)
    {
        if (table->entries[offset + index].hash == hash)
        {
            *out_nonce = table->entries[offset + index].nonce;
            return 1;
        }
        index = (index + 1) & PARTITION_SIZE_MASK;
    }
    return 0;
}

int find_collision(PdfFile file_a, PdfFile file_b,
                   uint64_t max_attempts,
                   uint64_t *found_nonce_a, uint64_t *found_nonce_b)
{
    PartitionedHashTable table;
    if (!table_init(&table))
    {
        printf("Error: could not allocate hash table\n");
        exit(1);
    }

    uint64_t phase1_limit = (TABLE_SIZE / 10) * 9;
    if (max_attempts < phase1_limit)
        phase1_limit = max_attempts;

    printf("Phase 1: filling table with hashes from file_a (parallel, up to %llu entries)...\n",
           (unsigned long long)phase1_limit);

    int insertion_failed = 0;

#pragma omp parallel
    {
        unsigned char *local_buf = malloc((size_t)file_a.size);
        if (local_buf == NULL)
        {
#pragma omp critical
            {
                fprintf(stderr, "Error: could not allocate thread-local file buffer\n");
                exit(1);
            }
        }
        memcpy(local_buf, file_a.data, (size_t)file_a.size);

#pragma omp for schedule(static)
        for (uint64_t nonce_a = 0; nonce_a < phase1_limit; nonce_a++)
        {
            int failed;
#pragma omp atomic read
            failed = insertion_failed;
            if (failed)
                continue;

            set_nonce(local_buf, nonce_a);
            uint64_t h = toy_hash(local_buf, (size_t)file_a.size);
            if (!table_insert(&table, h, nonce_a))
            {
                fprintf(stderr, "Error: hash table partition became full\n");
#pragma omp atomic write
                insertion_failed = 1;
            }
        }
        free(local_buf);
    }

    if (insertion_failed)
    {
        table_destroy(&table);
        return 0;
    }

    printf("Phase 2: searching file_b against the table (parallel)...\n");

    int found_flag = 0;
    uint64_t result_nonce_a = 0, result_nonce_b = 0;

#pragma omp parallel
    {
        unsigned char *local_buf = malloc((size_t)file_b.size);
        if (local_buf == NULL)
        {
#pragma omp critical
            {
                fprintf(stderr, "Error: could not allocate thread-local file buffer\n");
                exit(1);
            }
        }
        memcpy(local_buf, file_b.data, (size_t)file_b.size);

#pragma omp for schedule(static)
        for (uint64_t nonce_b = 0; nonce_b < max_attempts; nonce_b++)
        {
            int stop;
#pragma omp atomic read
            stop = found_flag;
            if (stop)
                continue;

            set_nonce(local_buf, nonce_b);
            uint64_t h = toy_hash(local_buf, (size_t)file_b.size);

            uint64_t matching_nonce_a;
            if (table_lookup(&table, h, &matching_nonce_a))
            {
#pragma omp critical
                {
                    int already_found;
#pragma omp atomic read
                    already_found = found_flag;
                    if (!already_found)
                    {
                        result_nonce_a = matching_nonce_a;
                        result_nonce_b = nonce_b;
#pragma omp atomic write
                        found_flag = 1;
                    }
                }
            }
        }
        free(local_buf);
    }

    table_destroy(&table);

    if (found_flag)
    {
        *found_nonce_a = result_nonce_a;
        *found_nonce_b = result_nonce_b;
        return 1;
    }
    return 0;
}

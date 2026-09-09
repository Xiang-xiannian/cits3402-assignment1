#include "collision.h"
#include "toy_hash.h"
#include <stdio.h>
#include <stdlib.h>

// Table size: about 33 million slots (2^25).
// This should comfortably hold the ~16 million entries
// we expect to insert before a collision is found.
#define TABLE_SIZE (1 << 25)

// Insert (hash, nonce) into the table using linear probing.
static void table_insert(HashEntry *table, uint64_t hash, uint64_t nonce)
{
    uint64_t index = hash % TABLE_SIZE;
    while (table[index].occupied)
    {
        index = (index + 1) % TABLE_SIZE;
    }
    table[index].hash = hash;
    table[index].nonce = nonce;
    table[index].occupied = 1;
}

// Look up a hash in the table. If found, return 1 and set *out_nonce.
// If not found, return 0.
static int table_lookup(HashEntry *table, uint64_t hash, uint64_t *out_nonce)
{
    uint64_t index = hash % TABLE_SIZE;
    while (table[index].occupied)
    {
        if (table[index].hash == hash)
        {
            *out_nonce = table[index].nonce;
            return 1;
        }
        index = (index + 1) % TABLE_SIZE;
    }
    return 0;
}

// int find_collision(PdfFile file_a, PdfFile file_b,
//                    uint64_t max_attempts,
//                    uint64_t *found_nonce_a, uint64_t *found_nonce_b)
// {
//     // Allocate the hash table. calloc zero-initialises it,
//     // so "occupied" starts as 0 for every slot.
//     HashEntry *table = calloc(TABLE_SIZE, sizeof(HashEntry));
//     if (table == NULL)
//     {
//         printf("Error: could not allocate hash table\n");
//         exit(1);
//     }

//     // Step 1: fill the table with hashes from file_a, trying
//     // nonce = 0, 1, 2, ... in order.
//     for (uint64_t nonce_a = 0; nonce_a < max_attempts; nonce_a++)
//     {
//         set_nonce(file_a.data, nonce_a);
//         uint64_t h = toy_hash(file_a.data, file_a.size);
//         table_insert(table, h, nonce_a);
//     }

//     // Step 2: try nonces on file_b, checking against the table
//     // each time. As soon as we get a hit, we have a collision.
//     for (uint64_t nonce_b = 0; nonce_b < max_attempts; nonce_b++)
//     {
//         set_nonce(file_b.data, nonce_b);
//         uint64_t h = toy_hash(file_b.data, file_b.size);

//         uint64_t matching_nonce_a;
//         if (table_lookup(table, h, &matching_nonce_a))
//         {
//             *found_nonce_a = matching_nonce_a;
//             *found_nonce_b = nonce_b;
//             free(table);
//             return 1; // success
//         }
//     }

//     free(table);
//     return 0; // gave up without finding a collision
// }

// just for testing
int find_collision(PdfFile file_a, PdfFile file_b,
                   uint64_t max_attempts,
                   uint64_t *found_nonce_a, uint64_t *found_nonce_b)
{
    HashEntry *table = calloc(TABLE_SIZE, sizeof(HashEntry));
    if (table == NULL)
    {
        printf("Error: could not allocate hash table\n");
        exit(1);
    }

    printf("Phase 1: filling table with hashes from file_a...\n");
    for (uint64_t nonce_a = 0; nonce_a < max_attempts; nonce_a++)
    {
        set_nonce(file_a.data, nonce_a);
        uint64_t h = toy_hash(file_a.data, file_a.size);
        table_insert(table, h, nonce_a);

        // Print progress every 1 million attempts
        if (nonce_a % 1000000 == 0 && nonce_a > 0)
        {
            printf("  ...%llu / %llu done\n",
                   (unsigned long long)nonce_a, (unsigned long long)max_attempts);
        }
    }

    printf("Phase 2: searching file_b against the table...\n");
    for (uint64_t nonce_b = 0; nonce_b < max_attempts; nonce_b++)
    {
        set_nonce(file_b.data, nonce_b);
        uint64_t h = toy_hash(file_b.data, file_b.size);

        uint64_t matching_nonce_a;
        if (table_lookup(table, h, &matching_nonce_a))
        {
            *found_nonce_a = matching_nonce_a;
            *found_nonce_b = nonce_b;
            free(table);
            return 1;
        }

        if (nonce_b % 1000000 == 0 && nonce_b > 0)
        {
            printf("  ...%llu / %llu done\n",
                   (unsigned long long)nonce_b, (unsigned long long)max_attempts);
        }
    }

    free(table);
    return 0;
}
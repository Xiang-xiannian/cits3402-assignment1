#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>
#include "pdf_io.h"
#include "toy_hash.h"
#include "collision.h"

#define STUDENT_NUMBER "25053306"
#define MAX_ATTEMPTS 300000000ULL

// Build an output filename like "solved_1_kilo_a.pdf" from an input path
// like "data/1_kilo_a.pdf", by stripping the directory and adding a prefix.
void build_output_name(const char *input_path, char *out, size_t out_size)
{
    const char *slash = strrchr(input_path, '/');
    const char *filename = slash ? slash + 1 : input_path;
    snprintf(out, out_size, "solved_%s", filename);
}
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s file_a.pdf file_b.pdf\n", argv[0]);
        return 1;
    }

    int num_threads = (argc == 4) ? atoi(argv[3]) : omp_get_max_threads();
    if (num_threads <= 0)
    {
        fprintf(stderr, "Error: thread count must be a positive integer.\n");
        return 1;
    }

    omp_set_dynamic(0);
    omp_set_num_threads(num_threads);
    printf("Using %d threads\n", num_threads);

    PdfFile file_a = pdf_load(argv[1]);
    PdfFile file_b = pdf_load(argv[2]);

    // Set my student number
    set_student_number(file_a.data, STUDENT_NUMBER);
    set_student_number(file_b.data, STUDENT_NUMBER);

    printf("Searching for a collision...\n");

    uint64_t nonce_a = 0, nonce_b = 0;

    /* Time only the search/collision-detection stage, excluding file I/O. */
    double start = omp_get_wtime();
    int found = find_collision(file_a, file_b, MAX_ATTEMPTS,
                               &nonce_a, &nonce_b);
    double end = omp_get_wtime();
    double seconds = end - start;

    if (!found)
    {
        printf("No collision found within %llu attempts (took %.3f seconds)\n",
               (unsigned long long)MAX_ATTEMPTS, seconds);
        free(file_a.data);
        free(file_b.data);
        return 1;
    }

    printf("Collision found! nonce_a=%llu, nonce_b=%llu (took %.3f seconds)\n",
           (unsigned long long)nonce_a,
           (unsigned long long)nonce_b,
           seconds);

    set_nonce(file_a.data, nonce_a);
    set_nonce(file_b.data, nonce_b);

    uint64_t check_a = toy_hash(file_a.data, (size_t)file_a.size);
    uint64_t check_b = toy_hash(file_b.data, (size_t)file_b.size);

    if (check_a != check_b)
    {
        printf("ERROR: verification failed! hashes do not match after all.\n");
        free(file_a.data);
        free(file_b.data);
        return 1;
    }

    printf("Verified: both files now hash to %012llx\n",
           (unsigned long long)check_a);

    char output_a[256];
    char output_b[256];
    build_output_name(argv[1], output_a, sizeof(output_a));
    build_output_name(argv[2], output_b, sizeof(output_b));

    pdf_write(output_a, file_a);
    pdf_write(output_b, file_b);
    printf("Wrote %s and %s\n", output_a, output_b);

    free(file_a.data);
    free(file_b.data);
    return 0;
}

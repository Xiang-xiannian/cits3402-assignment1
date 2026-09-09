#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pdf_io.h"
#include "toy_hash.h"
#include "collision.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s file_a.pdf file_b.pdf\n", argv[0]);
        return 1;
    }

    PdfFile file_a = pdf_load(argv[1]);
    PdfFile file_b = pdf_load(argv[2]);

    // Set my student number
    set_student_number(file_a.data, "12345678");
    set_student_number(file_b.data, "12345678");

    printf("Searching for a collision...\n");

    uint64_t nonce_a, nonce_b;
    uint64_t max_attempts = 30000000; // start small, increase later for harder pairs

    clock_t start = clock();
    int found = find_collision(file_a, file_b, max_attempts, &nonce_a, &nonce_b);
    clock_t end = clock();

    double seconds = (double)(end - start) / CLOCKS_PER_SEC;

    if (!found)
    {
        printf("No collision found within %llu attempts (took %.3f seconds)\n",
               (unsigned long long)max_attempts, seconds);
        free(file_a.data);
        free(file_b.data);
        return 1;
    }

    printf("Collision found! nonce_a=%llu, nonce_b=%llu (took %.3f seconds)\n",
           (unsigned long long)nonce_a, (unsigned long long)nonce_b, seconds);

    // Write the final nonces into the files
    set_nonce(file_a.data, nonce_a);
    set_nonce(file_b.data, nonce_b);

    // Verify: recompute the hashes from scratch to make sure they really match
    uint64_t check_a = toy_hash(file_a.data, file_a.size);
    uint64_t check_b = toy_hash(file_b.data, file_b.size);

    if (check_a != check_b)
    {
        printf("ERROR: verification failed! hashes do not match after all.\n");
        free(file_a.data);
        free(file_b.data);
        return 1;
    }

    printf("Verified: both files now hash to %012llx\n", (unsigned long long)check_a);

    pdf_write("solved_a.pdf", file_a);
    pdf_write("solved_b.pdf", file_b);
    printf("Wrote solved_a.pdf and solved_b.pdf\n");

    free(file_a.data);
    free(file_b.data);
    return 0;
}
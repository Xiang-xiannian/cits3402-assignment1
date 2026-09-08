#include <stdio.h>
#include <stdlib.h>
#include "pdf_io.h"
#include "toy_hash.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s file_a.pdf file_b.pdf\n", argv[0]);
        return 1;
    }

    // Load both files into memory
    PdfFile file_a = pdf_load(argv[1]);
    PdfFile file_b = pdf_load(argv[2]);

    printf("Loaded %s: %ld bytes\n", argv[1], file_a.size);
    printf("Loaded %s: %ld bytes\n", argv[2], file_b.size);

    // Check the hash matches what check_toy_hash.py gives
    uint64_t hash_a = toy_hash(file_a.data, file_a.size);
    uint64_t hash_b = toy_hash(file_b.data, file_b.size);
    printf("hash of file_a: %012llx\n", (unsigned long long)hash_a);
    printf("hash of file_b: %012llx\n", (unsigned long long)hash_b);

    // Set student number
    set_student_number(file_a.data, "12345678");
    set_student_number(file_b.data, "12345678");

    // Try setting a nonce and see the hash change
    set_nonce(file_a.data, 42);
    uint64_t hash_a2 = toy_hash(file_a.data, file_a.size);
    printf("hash of file_a after nonce=42: %012llx\n", (unsigned long long)hash_a2);

    // Write out a test file to check the bytes look right
    pdf_write("test_output.pdf", file_a);
    printf("Wrote test_output.pdf\n");

    free(file_a.data);
    free(file_b.data);

    return 0;
}
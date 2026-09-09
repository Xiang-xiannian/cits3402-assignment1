#include <stdio.h>
#include "pdf_io.h"
#include "toy_hash.h"

int main(void)
{
    PdfFile file = pdf_load("data/solved_example_a.pdf");
    uint64_t h = toy_hash(file.data, file.size);
    printf("hash = %012llx\n", (unsigned long long)h);
    printf("expected = 18a3ada27832\n");
    return 0;
}
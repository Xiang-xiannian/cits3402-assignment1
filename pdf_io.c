#include "pdf_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#define NONCE_OFFSET 16
#define NONCE_LENGTH 16
#define STUDENT_ID_OFFSET 45
#define STUDENT_ID_LENGTH 8
PdfFile pdf_load(const char *filepath)
{
    FILE *fp = fopen(filepath, "rb"); // "rb" = read binary, important for PDF files
    if (fp == NULL)
    {
        printf("Error: could not open %s\n", filepath);
        exit(1);
    }

    // Find out how big the file is
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET); // go back to the start

    // Allocate memory and read the whole file into it
    unsigned char *buffer = malloc(filesize);
    fread(buffer, 1, filesize, fp);
    fclose(fp);

    PdfFile pdf;
    pdf.data = buffer;
    pdf.size = filesize;
    return pdf;
}

void pdf_write(const char *filepath, PdfFile pdf)
{
    FILE *fp = fopen(filepath, "wb"); // "wb" = write binary
    fwrite(pdf.data, 1, pdf.size, fp);
    fclose(fp);
}

void set_nonce(unsigned char *pdf, uint64_t nonce)
{
    static const char hex[] = "0123456789abcdef";
    for (int i = NONCE_LENGTH - 1; i >= 0; --i)
    {
        pdf[NONCE_OFFSET + i] = (unsigned char)hex[nonce & 0xf];
        nonce >>= 4;
    }
}
void set_student_number(unsigned char *pdf, const char *id)
{
    memcpy(pdf + STUDENT_ID_OFFSET, id, STUDENT_ID_LENGTH);
}

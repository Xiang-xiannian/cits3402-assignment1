#ifndef PDF_IO_H
#define PDF_IO_H

#include <stdint.h>

// A struct to hold a PDF's bytes and its length together
typedef struct {
    unsigned char *data;
    long size;
} PdfFile;

// Read a PDF file from disk into memory
PdfFile pdf_load(const char *filepath);

// Write a PdfFile struct back to disk
void pdf_write(const char *filepath, PdfFile pdf);

// Overwrite the 16-character nonce field (bytes 16-31)
void set_nonce(unsigned char *pdf, uint64_t nonce);

// Overwrite the 8-character student ID field (bytes 45-52)
void set_student_number(unsigned char *pdf, const char *id);

#endif
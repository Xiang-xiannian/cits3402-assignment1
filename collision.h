#ifndef COLLISION_H
#define COLLISION_H

#include <stdint.h>
#include "pdf_io.h"

// One slot in our simple hash table
typedef struct
{
    uint64_t hash;
    uint64_t nonce;
    int occupied; // 0 = empty, 1 = filled
} HashEntry;

// Try to find a nonce pair (nonce_a, nonce_b) such that
// toy_hash(file_a with nonce_a) == toy_hash(file_b with nonce_b).
// Returns 1 if found, 0 if gave up after max_attempts.
int find_collision(PdfFile file_a, PdfFile file_b,
                   uint64_t max_attempts,
                   uint64_t *found_nonce_a, uint64_t *found_nonce_b);

#endif
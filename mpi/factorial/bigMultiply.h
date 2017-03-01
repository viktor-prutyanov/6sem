#ifndef BIGMULTIPLY_H
#define BIGMULTIPLY_H

#include <stdio.h>
#include <stdint.h>

uint32_t *treeMultiply(size_t *ans_len, unsigned int start, unsigned int end);
uint32_t *bigMultiply(size_t *len_c, uint32_t *a, uint32_t *b, size_t len_a, size_t len_b);
void fdump(FILE *file, uint32_t *a, size_t len);

#endif /* BIGMULTIPLY_H */

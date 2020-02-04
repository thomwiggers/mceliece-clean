#ifndef MC_VEC_H
#define MC_VEC_H

#include <stdint.h>

extern void MC_vec_mul_asm(uint64_t *, const uint64_t *, const uint64_t *, int);

void MC_vec_mul(uint64_t *h, const uint64_t *f, const uint64_t *g);
void MC_vec_add(uint64_t *h, const uint64_t *f, const uint64_t *g);

#endif

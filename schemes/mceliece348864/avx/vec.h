#ifndef MC_VEC_H
#define MC_VEC_H

#include <stdint.h>


void MC_vec_mul(uint64_t *h, const uint64_t *f, const uint64_t *g);

void MC_vec_mul_sp(uint64_t *h, const uint64_t *f, const uint64_t *g);

void MC_vec_add(uint64_t *h, const uint64_t *f, const uint64_t *g);

#endif

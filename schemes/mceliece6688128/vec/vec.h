#ifndef MC_VEC_H
#define MC_VEC_H

#include "params.h"

#include <stdint.h>

typedef uint64_t vec;

vec MC_vec_setbits(vec b);

vec MC_vec_set1_16b(uint16_t v);

void MC_vec_copy(vec *out, const vec *in);

vec MC_vec_or_reduce(const vec *a);

int MC_vec_testz(vec a);

void MC_vec_mul(vec *, const vec *, const vec *);
void MC_vec_sq(vec *, const vec *);
void MC_vec_inv(vec *, const vec *);

#endif


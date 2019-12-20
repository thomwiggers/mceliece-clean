#ifndef GF_H
#define GF_H
/*
  This file is for functions for field arithmetic
*/


#include "params.h"

#include <stdint.h>

typedef uint16_t gf;

gf MC_gf_iszero(gf);
gf MC_gf_mul(gf, gf);
gf MC_gf_frac(gf, gf);
gf MC_gf_inv(gf);

void MC_GF_mul(gf *, const gf *, const gf *);

/* 2 field multiplications */
uint64_t MC_gf_mul2(gf a, gf b0, gf b1);

#endif


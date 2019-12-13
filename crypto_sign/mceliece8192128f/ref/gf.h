/*
  This file is for functions for field arithmetic
*/

#ifndef GF_H
#define GF_H

#include <stdint.h>

typedef uint16_t gf;

gf MC_gf_iszero(gf);
gf MC_gf_add(gf, gf);
gf MC_gf_mul(gf, gf);
gf MC_gf_frac(gf, gf);
gf MC_gf_inv(gf);

void MC_GF_mul(gf *, gf *, gf *);

#endif


#ifndef MC_GF_H
#define MC_GF_H
/*
  This file is for functions for field arithmetic
*/


#include <stdint.h>

typedef uint16_t gf;

gf MC_gf_iszero(gf);
gf MC_gf_add(gf, gf);
gf MC_gf_mul(gf, gf);
gf MC_gf_frac(gf, gf);
gf MC_gf_inv(gf);

void MC_GF_mul(gf *, const gf *, const gf *);

#endif


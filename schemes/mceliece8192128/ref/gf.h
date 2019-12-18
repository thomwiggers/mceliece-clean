#ifndef MC_GF_H
#define MC_GF_H
/*
  This file is for functions for field arithmetic
*/


#include <stdint.h>

typedef uint16_t gf;

gf MC_gf_iszero(gf a);
gf MC_gf_add(gf in0, gf in1);
gf MC_gf_mul(gf in0, gf in1);
gf MC_gf_frac(gf den, gf num);
gf MC_gf_inv(gf in);

void MC_GF_mul(gf *out, const gf *in0, const gf *in1);

#endif


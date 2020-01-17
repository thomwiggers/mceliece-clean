#ifndef MC_SK_GEN_H
#define MC_SK_GEN_H
/*
  This file is for secret-key generation
*/


#include "gf.h"

#include <stdint.h>

int MC_genpoly_gen(gf *, gf *);
int MC_perm_check(const uint32_t *);

#endif


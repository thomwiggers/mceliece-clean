#ifndef MC_BM_H
#define MC_BM_H
/*
  This file is for the inversion-free Berlekamp-Massey algorithm
  see https://ieeexplore.ieee.org/document/87857
*/


#include <stdint.h>

#include "params.h"
#include "vec128.h"

void MC_bm(uint64_t *out, vec128 *in);

#endif


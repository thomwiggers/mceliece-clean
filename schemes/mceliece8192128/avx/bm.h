#ifndef BM_H
#define BM_H
/*
  This file is for the inversion-free Berlekamp-Massey algorithm
  see https://ieeexplore.ieee.org/document/87857
*/

#include "vec128.h"
#include "vec256.h"

void MC_bm(vec128 *, vec256 *);

#endif


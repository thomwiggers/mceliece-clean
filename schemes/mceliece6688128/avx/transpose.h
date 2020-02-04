#ifndef MC_TRANSPOSE_H
#define MC_TRANSPOSE_H
/*
  This file is for matrix transposition
*/


#include "vec128.h"
#include "vec256.h"

void MC_transpose_64x128_sp(vec128 *in);
void MC_transpose_64x256_sp(vec256 *in);

#endif



#ifndef MC_TRANSPOSE_H
#define MC_TRANSPOSE_H
/*
  This file is for matrix transposition
*/


#include "vec128.h"

#include <stdint.h>

void MC_transpose_64x64(uint64_t *in);
void MC_transpose_64x128_sp(vec128 *in);

#endif


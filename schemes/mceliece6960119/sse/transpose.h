#ifndef MC_TRANSPOSE_H
#define MC_TRANSPOSE_H
/*
  This file is for matrix transposition
*/


#include "vec128.h"

void MC_transpose_64x128_sp_asm(vec128 *);

inline static void MC_transpose_64x128_sp(vec128 *in) {
    MC_transpose_64x128_sp_asm(in);
}

#endif


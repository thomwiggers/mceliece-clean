/*
  This file is for matrix transposition
*/

#include "transpose.h"

extern void MC_transpose_64x128_sp_asm(vec128 *);

void MC_transpose_64x128_sp(vec128 *in) {
    MC_transpose_64x128_sp_asm(in);
}

extern void MC_transpose_64x256_sp_asm(vec256 *);

void MC_transpose_64x256_sp(vec256 *in) {
    MC_transpose_64x256_sp_asm(in);
}


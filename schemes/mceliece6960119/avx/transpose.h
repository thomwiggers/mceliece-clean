#ifndef TRANSPOSE_H
#define TRANSPOSE_H
/*
  This file is for matrix transposition
*/


#include "vec128.h"
#include "vec256.h"

extern void MC_transpose_64x128_sp_asm(vec128 *);

static inline void MC_transpose_64x128_sp(vec128 *in) {
    MC_transpose_64x128_sp_asm(in);
}

extern void MC_transpose_64x256_sp_asm(vec256 *);

static inline void MC_transpose_64x256_sp(vec256 *in) {
    MC_transpose_64x256_sp_asm(in);
}

#endif



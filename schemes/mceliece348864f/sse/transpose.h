#ifndef MC_TRANSPOSE_H
#define MC_TRANSPOSE_H
/*
  This file is for matrix transposition
*/


#include "vec128.h"

#include <stdint.h>

extern void MC_transpose_64x64_asm(uint64_t *);

static inline void MC_transpose_64x64(uint64_t *in) {
    MC_transpose_64x64_asm(in);
}

extern void MC_transpose_64x128_sp_asm(vec128 *);

static inline void MC_transpose_64x128_sp(vec128 *in) {
    MC_transpose_64x128_sp_asm(in);
}

#endif


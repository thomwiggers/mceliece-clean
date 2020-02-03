#ifndef MC_TRANSPOSE_H
#define MC_TRANSPOSE_H
/*
  This file is for matrix transposition
*/


#include "vec256.h"

#include <stdint.h>

extern void MC_transpose_64x64_asm(uint64_t *);

static inline void MC_transpose_64x64(uint64_t *in) {
    MC_transpose_64x64_asm(in);
}

extern void MC_transpose_64x256_sp_asm(vec256 *);

static inline void MC_transpose_64x256_sp(vec256 *in) {
    MC_transpose_64x256_sp_asm(in);
}

#endif


#include "transpose.h"

/*
  This file is for matrix transposition
*/

extern void MC_transpose_64x64_asm(uint64_t *);
extern void MC_transpose_64x256_sp_asm(vec256 *);


void MC_transpose_64x64(uint64_t *in) {
    MC_transpose_64x64_asm(in);
}

void MC_transpose_64x256_sp(vec256 *in) {
    MC_transpose_64x256_sp_asm(in);
}

#include "transpose.h"

extern void MC_transpose_64x64_asm(uint64_t *);
void MC_transpose_64x64(uint64_t *in) {
    MC_transpose_64x64_asm(in);
}

extern void MC_transpose_64x128_sp_asm(vec128 *);

void MC_transpose_64x128_sp(vec128 *in) {
    MC_transpose_64x128_sp_asm(in);
}

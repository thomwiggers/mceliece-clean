
#include "vec.h"

#include "params.h"

void MC_vec_mul(uint64_t *h, const uint64_t *f, const uint64_t *g) {
    MC_vec_mul_asm(h, f, g, 8);
}

void MC_vec_add(uint64_t *h, const uint64_t *f, const uint64_t *g) {
    int b;

    for (b = 0; b < GFBITS; b++) {
        h[b] = f[b] ^ g[b];
    }
}


#include "params.h"

#include <stdint.h>

extern void MC_vec_mul_asm(uint64_t *, const uint64_t *, const uint64_t *);

static inline void vec_mul(uint64_t *h, const uint64_t *f, const uint64_t *g) {
    MC_vec_mul_asm(h, f, g);
}

extern void MC_vec_mul_sp_asm(uint64_t *, const uint64_t *, const uint64_t *);

static inline void vec_mul_sp(uint64_t *h, const uint64_t *f, const uint64_t *g) {
    MC_vec_mul_sp_asm(h, f, g);
}

static inline void vec_add(uint64_t *h, const uint64_t *f, const uint64_t *g) {
    int b;

    for (b = 0; b < GFBITS; b++) {
        h[b] = f[b] ^ g[b];
    }
}


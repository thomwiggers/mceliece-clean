/*
  This file is for Niederreiter encryption
*/

#include "encrypt.h"

#include "randombytes.h"
#include "params.h"
#include "util.h"

#include <stdint.h>

/* input: public key pk, error vector e */
/* output: syndrome s */
extern void MC_syndrome_asm(unsigned char *s, const unsigned char *pk, unsigned char *e);

/* output: e, an error vector of weight t */
static void gen_e(unsigned char *e) {
    size_t i, j;
    int eq;

    uint16_t ind[ SYS_T ];
    uint64_t e_int[ SYS_N / 64 ];
    uint64_t one = 1;
    uint64_t mask;
    uint64_t val[ SYS_T ];

    while (1) {
        randombytes((uint8_t*)ind, sizeof(ind));

        for (i = 0; i < SYS_T; i++) {
            ind[i] &= GFMASK;
        }

        for (i = 0; i < SYS_T; i++) {
            ind[i] &= GFMASK;
        }

        // check for repetition

        eq = 0;

        for (i = 1; i < SYS_T; i++) for (j = 0; j < i; j++)
                if (ind[i] == ind[j]) {
                    eq = 1;
                }

        if (eq == 0) {
            break;
        }
    }

    for (j = 0; j < SYS_T; j++) {
        val[j] = one << (ind[j] & 63);
    }

    for (i = 0; i < SYS_N / 64; i++) {
        e_int[i] = 0;

        for (j = 0; j < SYS_T; j++) {
            mask = i ^ (ind[j] >> 6);
            mask -= 1;
            mask >>= 63;
            mask = -mask;

            e_int[i] |= val[j] & mask;
        }
    }

    for (i = 0; i < SYS_N / 64; i++) {
        MC_store8(e + i * 8, e_int[i]);
    }
}

/* input: public key pk */
/* output: error vector e, syndrome s */
void MC_encrypt(unsigned char *s, unsigned char *e, const unsigned char *pk) {
    gen_e(e);
    MC_syndrome_asm(s, pk, e);
}


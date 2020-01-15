/*
  This file is for Niederreiter decryption
*/

#include "decrypt.h"

#include "params.h"
#include "fft_tr.h"
#include "benes.h"
#include "util.h"
#include "fft.h"
#include "vec.h"
#include "bm.h"

#include <stdio.h>

static void scaling(vec out[][GFBITS], vec inv[][GFBITS], const unsigned char *sk, const vec *recv) {
    int i, j;

    vec irr_int[2][ GFBITS ];
    vec eval[128][ GFBITS ];
    vec tmp[ GFBITS ];

    //

    MC_irr_load(irr_int, sk);

    MC_fft(eval, irr_int);

    for (i = 0; i < 128; i++) {
        MC_vec_sq(eval[i], eval[i]);
    }

    MC_vec_copy(inv[0], eval[0]);

    for (i = 1; i < 128; i++) {
        MC_vec_mul(inv[i], inv[i - 1], eval[i]);
    }

    MC_vec_inv(tmp, inv[127]);

    for (i = 126; i >= 0; i--) {
        MC_vec_mul(inv[i + 1], tmp, inv[i]);
        MC_vec_mul(tmp, tmp, eval[i + 1]);
    }

    MC_vec_copy(inv[0], tmp);

    //

    for (i = 0; i < 128; i++)
        for (j = 0; j < GFBITS; j++) {
            out[i][j] = inv[i][j] & recv[i];
        }
}

static void scaling_inv(vec out[][GFBITS], vec inv[][GFBITS], const vec *recv) {
    int i, j;

    for (i = 0; i < 128; i++)
        for (j = 0; j < GFBITS; j++) {
            out[i][j] = inv[i][j] & recv[i];
        }
}

static void preprocess(vec *recv, const unsigned char *s) {
    int i;

    recv[0] = 0;

    for (i = 1; i < 128; i++) {
        recv[i] = recv[0];
    }

    for (i = 0; i < SYND_BYTES / 8; i++) {
        recv[i] = MC_load8(s + i * 8);
    }
}

static int weight(const vec *v) {
    int i, w = 0;

    for (i = 0; i < SYS_N; i++) {
        w += (int)((v[i / 64] >> (i % 64)) & 1);
    }

    return w;
}

static uint16_t synd_cmp(vec s0[][ GFBITS ], vec s1[][ GFBITS ]) {
    int i, j;
    vec diff = 0;

    for (i = 0; i < 4; i++)
        for (j = 0; j < GFBITS; j++) {
            diff |= (s0[i][j] ^ s1[i][j]);
        }

    return MC_vec_testz(diff);
}

/* Niederreiter decryption with the Berlekamp decoder */
/* intput: sk, secret key */
/*         s, ciphertext (syndrome) */
/* output: e, error vector */
/* return: 0 for success; 1 for failure */
int MC_decrypt(unsigned char *e, const unsigned char *sk, const unsigned char *c) {
    int i;

    uint16_t check_synd;
    uint16_t check_weight;

    vec inv[ 128 ][ GFBITS ];
    vec scaled[ 128 ][ GFBITS ];
    vec eval[ 128 ][ GFBITS ];

    vec error[ 128 ];

    vec s_priv[ 4 ][ GFBITS ];
    vec s_priv_cmp[ 4 ][ GFBITS ];
    vec locator[2][ GFBITS ];

    vec recv[ 128 ];
    vec allone;

    // Berlekamp decoder

    preprocess(recv, c);

    MC_benes(recv, sk + IRR_BYTES, 1);
    scaling(scaled, inv, sk, recv);
    MC_fft_tr(s_priv, scaled);
    MC_bm(locator, s_priv);

    MC_fft(eval, locator);

    // reencryption and weight check

    allone = MC_vec_setbits(1);

    for (i = 0; i < 128; i++) {
        error[i] = MC_vec_or_reduce(eval[i]);
        error[i] ^= allone;
    }

    check_weight = weight(error) ^ SYS_T;
    check_weight -= 1;
    check_weight >>= 15;

    scaling_inv(scaled, inv, error);
    MC_fft_tr(s_priv_cmp, scaled);

    check_synd = synd_cmp(s_priv, s_priv_cmp);

    //

    MC_benes(error, sk + IRR_BYTES, 0);

    for (i = 0; i < 128; i++) {
        MC_store8(e + i * 8, error[i]);
    }

    return 1 - (check_synd & check_weight);
}


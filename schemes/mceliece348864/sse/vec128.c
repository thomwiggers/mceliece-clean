/*
  This file is for functions related to 128-bit vectors
  including functions for bitsliced field operations
*/

#include "vec128.h"

/* bitsliced field squarings */
void MC_vec128_sq(vec128 *out, vec128 *in) {
    int i;
    vec128 result[GFBITS];

    result[0] = in[0] ^ in[6];
    result[1] = in[11];
    result[2] = in[1] ^ in[7];
    result[3] = in[6];
    result[4] = in[2] ^ in[11] ^ in[8];
    result[5] = in[7];
    result[6] = in[3] ^ in[9];
    result[7] = in[8];
    result[8] = in[4] ^ in[10];
    result[9] = in[9];
    result[10] = in[5] ^ in[11];
    result[11] = in[10];

    for (i = 0; i < GFBITS; i++) {
        out[i] = result[i];
    }
}

/* bitsliced field inverses */
void MC_vec128_inv(vec128 *out, vec128 *in) {
    vec128 tmp_11[ GFBITS ];
    vec128 tmp_1111[ GFBITS ];

    vec128_copy(out, in);

    MC_vec128_sq(out, out);
    vec128_mul(tmp_11, out, in); // 11

    MC_vec128_sq(out, tmp_11);
    MC_vec128_sq(out, out);
    vec128_mul(tmp_1111, out, tmp_11); // 1111

    MC_vec128_sq(out, tmp_1111);
    MC_vec128_sq(out, out);
    MC_vec128_sq(out, out);
    MC_vec128_sq(out, out);
    vec128_mul(out, out, tmp_1111); // 11111111

    MC_vec128_sq(out, out);
    MC_vec128_sq(out, out);
    vec128_mul(out, out, tmp_11); // 1111111111

    MC_vec128_sq(out, out);
    vec128_mul(out, out, in); // 11111111111

    MC_vec128_sq(out, out); // 111111111110
}


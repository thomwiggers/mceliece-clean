/*
  This file is for the Gao-Mateer FFT
  sse http://www.math.clemson.edu/~sgao/papers/GM10.pdf
*/

#include "fft.h"

#include "transpose.h"
#include "vec128.h"

#include <stdint.h>

/* input: in, polynomial in bitsliced form */
/* output: in, result of applying the radix conversions on in */
static void radix_conversions(vec128 *in) {
    int i, j, k;
    vec128 t;
    uint64_t v0, v1;

    const vec128 mask[5][2] = {
        {
            MC_vec128_set2x(0x8888888888888888, 0x8888888888888888),
            MC_vec128_set2x(0x4444444444444444, 0x4444444444444444)
        },
        {
            MC_vec128_set2x(0xC0C0C0C0C0C0C0C0, 0xC0C0C0C0C0C0C0C0),
            MC_vec128_set2x(0x3030303030303030, 0x3030303030303030)
        },
        {
            MC_vec128_set2x(0xF000F000F000F000, 0xF000F000F000F000),
            MC_vec128_set2x(0x0F000F000F000F00, 0x0F000F000F000F00)
        },
        {
            MC_vec128_set2x(0xFF000000FF000000, 0xFF000000FF000000),
            MC_vec128_set2x(0x00FF000000FF0000, 0x00FF000000FF0000)
        },
        {
            MC_vec128_set2x(0xFFFF000000000000, 0xFFFF000000000000),
            MC_vec128_set2x(0x0000FFFF00000000, 0x0000FFFF00000000)
        }
    };

    const vec128 s[5][GFBITS] = {
#include "scalars_2x.inc"
    };

    //

    for (j = 0; j <= 5; j++) {
        for (i = 0; i < GFBITS; i++) {
            v1 = MC_vec128_extract(in[i], 1);
            v1 ^= v1 >> 32;
            v0 = MC_vec128_extract(in[i], 0);
            v0 ^= v1 << 32;
            in[i] = MC_vec128_set2x(v0, v1);
        }

        for (i = 0; i < GFBITS; i++)
            for (k = 4; k >= j; k--) {
                t = MC_vec128_and(in[i], mask[k][0]);
                t = MC_vec128_srl_2x(t, 1 << k);
                in[i] = MC_vec128_xor(in[i], t);

                t = MC_vec128_and(in[i], mask[k][1]);
                t = MC_vec128_srl_2x(t, 1 << k);
                in[i] = MC_vec128_xor(in[i], t);
            }

        if (j < 5) {
            MC_vec128_mul(in, in, s[j]);
        }
    }
}

/* input: in, result of applying the radix conversions to the input polynomial */
/* output: out, evaluation results (by applying the FFT butterflies) */
static void butterflies(vec128 out[][ GFBITS ], const vec128 *in) {
    int i, j, k, s, b;

    vec128 tmp[ GFBITS ];
    vec128 t[ GFBITS ];
    vec128 pre[8][ GFBITS ];
    vec128 buf[64];

    uint64_t v0, v1;
    uint64_t consts_ptr = 1;

    const vec128 consts[ 64 ][ GFBITS ] = {
#include "consts.inc"
    };

    const vec128 powers[ 64 ][ GFBITS ] = {
#include "powers.inc"
    };

    const unsigned char reversal[64] = {
        0, 32, 16, 48,  8, 40, 24, 56,
        4, 36, 20, 52, 12, 44, 28, 60,
        2, 34, 18, 50, 10, 42, 26, 58,
        6, 38, 22, 54, 14, 46, 30, 62,
        1, 33, 17, 49,  9, 41, 25, 57,
        5, 37, 21, 53, 13, 45, 29, 61,
        3, 35, 19, 51, 11, 43, 27, 59,
        7, 39, 23, 55, 15, 47, 31, 63
    };

    const uint16_t beta[8] = {2522, 7827, 7801, 8035, 6897, 8167, 3476, 0};

    //

    for (j = 0; j < GFBITS; j++) {
        t[j] = MC_vec128_unpack_high(in[j], in[j]);
    }

    for (i = 0; i < 8; i += 2) {
        for (j = 0; j < GFBITS; j++) {
            v0 = (beta[i + 0] >> j) & 1;
            v0 = -v0;
            v1 = (beta[i + 1] >> j) & 1;
            v1 = -v1;

            tmp[j] = MC_vec128_set2x(v0, v1);
        }

        MC_vec128_mul(tmp, t, tmp);

        for (j = 0; j < GFBITS; j++) {
            pre[i + 0][j] = MC_vec128_unpack_low(tmp[j], tmp[j]);
            pre[i + 1][j] = MC_vec128_unpack_high(tmp[j], tmp[j]);
        }
    }

    for (i = 0; i < GFBITS; i++) {
        buf[0] = MC_vec128_set2x(MC_vec128_extract(in[i], 0),
                              MC_vec128_extract(in[i], 0) ^ MC_vec128_extract(pre[6][i], 0));

        buf[1] = MC_vec128_xor(buf[0], pre[0][i]);
        buf[16] = MC_vec128_xor(buf[0], pre[4][i]);
        buf[3] = MC_vec128_xor(buf[1], pre[1][i]);
        buf[48] = MC_vec128_xor(buf[16], pre[5][i]);
        buf[49] = MC_vec128_xor(buf[48], pre[0][i]);
        buf[2] = MC_vec128_xor(buf[0], pre[1][i]);
        buf[51] = MC_vec128_xor(buf[49], pre[1][i]);
        buf[6] = MC_vec128_xor(buf[2], pre[2][i]);
        buf[50] = MC_vec128_xor(buf[51], pre[0][i]);
        buf[7] = MC_vec128_xor(buf[6], pre[0][i]);
        buf[54] = MC_vec128_xor(buf[50], pre[2][i]);
        buf[5] = MC_vec128_xor(buf[7], pre[1][i]);
        buf[55] = MC_vec128_xor(buf[54], pre[0][i]);
        buf[53] = MC_vec128_xor(buf[55], pre[1][i]);
        buf[4] = MC_vec128_xor(buf[0], pre[2][i]);
        buf[52] = MC_vec128_xor(buf[53], pre[0][i]);
        buf[12] = MC_vec128_xor(buf[4], pre[3][i]);
        buf[60] = MC_vec128_xor(buf[52], pre[3][i]);
        buf[13] = MC_vec128_xor(buf[12], pre[0][i]);
        buf[61] = MC_vec128_xor(buf[60], pre[0][i]);
        buf[15] = MC_vec128_xor(buf[13], pre[1][i]);
        buf[63] = MC_vec128_xor(buf[61], pre[1][i]);
        buf[14] = MC_vec128_xor(buf[15], pre[0][i]);
        buf[62] = MC_vec128_xor(buf[63], pre[0][i]);
        buf[10] = MC_vec128_xor(buf[14], pre[2][i]);
        buf[58] = MC_vec128_xor(buf[62], pre[2][i]);
        buf[11] = MC_vec128_xor(buf[10], pre[0][i]);
        buf[59] = MC_vec128_xor(buf[58], pre[0][i]);
        buf[9] = MC_vec128_xor(buf[11], pre[1][i]);
        buf[57] = MC_vec128_xor(buf[59], pre[1][i]);
        buf[56] = MC_vec128_xor(buf[57], pre[0][i]);
        buf[8] = MC_vec128_xor(buf[0], pre[3][i]);
        buf[40] = MC_vec128_xor(buf[56], pre[4][i]);
        buf[24] = MC_vec128_xor(buf[8], pre[4][i]);
        buf[41] = MC_vec128_xor(buf[40], pre[0][i]);
        buf[25] = MC_vec128_xor(buf[24], pre[0][i]);
        buf[43] = MC_vec128_xor(buf[41], pre[1][i]);
        buf[27] = MC_vec128_xor(buf[25], pre[1][i]);
        buf[42] = MC_vec128_xor(buf[43], pre[0][i]);
        buf[26] = MC_vec128_xor(buf[27], pre[0][i]);
        buf[46] = MC_vec128_xor(buf[42], pre[2][i]);
        buf[30] = MC_vec128_xor(buf[26], pre[2][i]);
        buf[47] = MC_vec128_xor(buf[46], pre[0][i]);
        buf[31] = MC_vec128_xor(buf[30], pre[0][i]);
        buf[45] = MC_vec128_xor(buf[47], pre[1][i]);
        buf[29] = MC_vec128_xor(buf[31], pre[1][i]);
        buf[44] = MC_vec128_xor(buf[45], pre[0][i]);
        buf[28] = MC_vec128_xor(buf[29], pre[0][i]);
        buf[36] = MC_vec128_xor(buf[44], pre[3][i]);
        buf[20] = MC_vec128_xor(buf[28], pre[3][i]);
        buf[37] = MC_vec128_xor(buf[36], pre[0][i]);
        buf[21] = MC_vec128_xor(buf[20], pre[0][i]);
        buf[39] = MC_vec128_xor(buf[37], pre[1][i]);
        buf[23] = MC_vec128_xor(buf[21], pre[1][i]);
        buf[38] = MC_vec128_xor(buf[39], pre[0][i]);
        buf[22] = MC_vec128_xor(buf[23], pre[0][i]);
        buf[34] = MC_vec128_xor(buf[38], pre[2][i]);
        buf[18] = MC_vec128_xor(buf[22], pre[2][i]);
        buf[35] = MC_vec128_xor(buf[34], pre[0][i]);
        buf[19] = MC_vec128_xor(buf[18], pre[0][i]);
        buf[33] = MC_vec128_xor(buf[35], pre[1][i]);
        buf[17] = MC_vec128_xor(buf[19], pre[1][i]);
        buf[32] = MC_vec128_xor(buf[33], pre[0][i]);

        MC_transpose_64x128_sp(buf);

        for (j = 0; j < 64; j++) {
            out[ reversal[j] ][i] = buf[j];
        }
    }

    for (i = 0; i <= 5; i++) {
        s = 1 << i;

        for (j = 0; j < 64; j += 2 * s)
            for (k = j; k < j + s; k++) {
                MC_vec128_mul(tmp, out[k + s], (vec128 *) consts[ consts_ptr + (k - j) ]);

                for (b = 0; b < GFBITS; b++) {
                    out[k  ][b] = MC_vec128_xor(out[k  ][b], tmp[b]);
                }
                for (b = 0; b < GFBITS; b++) {
                    out[k + s][b] = MC_vec128_xor(out[k + s][b], out[k][b]);
                }
            }

        consts_ptr += (1 << i);
    }

    // adding the part contributed by x^128

    for (i = 0; i < 64; i++)
        for (b = 0; b < GFBITS; b++) {
            out[i][b] = MC_vec128_xor(out[i][b], powers[i][b]);
        }
}

/* input: in, polynomial in bitsliced form */
/* output: out, bitsliced results of evaluating in all the field elements */
void MC_fft(vec128 out[][GFBITS], vec128 *in) {
    radix_conversions(in);
    butterflies(out, in);
}


/*
  This file is for transpose of the Gao-Mateer FFT
  Functions with names ending with _tr are (roughly) the transpose of the corresponding functions in fft.c
*/

#include "fft_tr.h"

#include "transpose.h"

#include <stdint.h>

static void radix_conversions_tr(vec256 *in) {
    int i, j, k;
    vec256 t;
    uint64_t v[4];

    const vec256 mask[6][2] = {
        {
            MC_vec256_set4x(0x2222222222222222, 0x2222222222222222, 0x2222222222222222, 0x2222222222222222),
            MC_vec256_set4x(0x4444444444444444, 0x4444444444444444, 0x4444444444444444, 0x4444444444444444)
        },
        {
            MC_vec256_set4x(0x0C0C0C0C0C0C0C0C, 0x0C0C0C0C0C0C0C0C, 0x0C0C0C0C0C0C0C0C, 0x0C0C0C0C0C0C0C0C),
            MC_vec256_set4x(0x3030303030303030, 0x3030303030303030, 0x3030303030303030, 0x3030303030303030)
        },
        {
            MC_vec256_set4x(0x00F000F000F000F0, 0x00F000F000F000F0, 0x00F000F000F000F0, 0x00F000F000F000F0),
            MC_vec256_set4x(0x0F000F000F000F00, 0x0F000F000F000F00, 0x0F000F000F000F00, 0x0F000F000F000F00)
        },
        {
            MC_vec256_set4x(0x0000FF000000FF00, 0x0000FF000000FF00, 0x0000FF000000FF00, 0x0000FF000000FF00),
            MC_vec256_set4x(0x00FF000000FF0000, 0x00FF000000FF0000, 0x00FF000000FF0000, 0x00FF000000FF0000)
        },
        {
            MC_vec256_set4x(0x00000000FFFF0000, 0x00000000FFFF0000, 0x00000000FFFF0000, 0x00000000FFFF0000),
            MC_vec256_set4x(0x0000FFFF00000000, 0x0000FFFF00000000, 0x0000FFFF00000000, 0x0000FFFF00000000)
        },
        {
            MC_vec256_set4x(0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000, 0xFFFFFFFF00000000),
            MC_vec256_set4x(0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF)
        }
    };

    const vec256 s[6][GFBITS] = {
#include "scalars_4x.inc"
    };

    //

    for (j = 6; j >= 0; j--) {
        if (j < 6) {
            MC_vec256_mul(in, in, s[j]); // scaling
        }

        for (k = j; k <= 4; k++)
            for (i = 0; i < GFBITS; i++) {
                t = MC_vec256_and(in[i], mask[k][0]);
                t = MC_vec256_sll_4x(t, 1 << k);
                in[i] = MC_vec256_xor(in[i], t);

                t = MC_vec256_and(in[i], mask[k][1]);
                t = MC_vec256_sll_4x(t, 1 << k);
                in[i] = MC_vec256_xor(in[i], t);
            }

        if (j <= 5)
            for (i = 0; i < GFBITS; i++) {
                v[0] = MC_vec256_extract(in[i], 0);
                v[1] = MC_vec256_extract(in[i], 1);
                v[2] = MC_vec256_extract(in[i], 2);
                v[3] = MC_vec256_extract(in[i], 3);

                v[1] ^= v[0] >> 32;
                v[1] ^= v[1] << 32;
                v[3] ^= v[2] >> 32;
                v[3] ^= v[3] << 32;

                in[i] = MC_vec256_set4x(v[0], v[1], v[2], v[3]);
            }

        for (i = 0; i < GFBITS; i++) {
            v[0] = MC_vec256_extract(in[i], 0);
            v[1] = MC_vec256_extract(in[i], 1);
            v[2] = MC_vec256_extract(in[i], 2);
            v[3] = MC_vec256_extract(in[i], 3);

            v[2] ^= v[1];
            v[3] ^= v[2];

            in[i] = MC_vec256_set4x(v[0], v[1], v[2], v[3]);
        }

    }
}

static void butterflies_tr(vec256 *out, vec256 in[][ GFBITS ]) {
    int i, j, k, s, b;

    vec256 t0[ GFBITS ];
    vec256 t1[ GFBITS ];
    vec256 t;

    vec128 out128[ GFBITS ][ 2 ];
    vec128 tmp[ GFBITS ];

    union {
        vec128 v[6][  GFBITS + 1    ];
        vec256 V[6][ (GFBITS + 1) / 2 ];
    } pre;

    union {
        vec128 v[64][ 2 ];
        vec256 V[64];
    } buf;

    const vec256 consts[ 33 ][ GFBITS ] = {
#include "consts.inc"
    };

    uint64_t v[4];
    uint64_t consts_ptr = 33;

    const unsigned char reversal[] = {
        0, 32, 16, 48,  8, 40, 24, 56,
        4, 36, 20, 52, 12, 44, 28, 60,
        2, 34, 18, 50, 10, 42, 26, 58,
        6, 38, 22, 54, 14, 46, 30, 62,
        1, 33, 17, 49,  9, 41, 25, 57,
        5, 37, 21, 53, 13, 45, 29, 61,
        3, 35, 19, 51, 11, 43, 27, 59,
        7, 39, 23, 55, 15, 47, 31, 63
    };

    const uint16_t beta[6] = {5246, 5306, 6039, 6685, 4905, 6755};

    // butterflies

    for (i = 4; i >= 0; i--) {
        s = 1 << i;
        consts_ptr -= s;

        for (j = 0; j < 32; j += 2 * s)
            for (k = j; k < j + s; k++) {
                MC_vec256_ama_asm(in[k], in[k + s], consts[ consts_ptr + (k - j) ]);
            }

    }

    for (k = 0; k < 32; k += 2) {
        for (b = 0; b < GFBITS; b++) {
            t0[b] = MC_vec256_unpack_low(in[k][b], in[k + 1][b]);
        }
        for (b = 0; b < GFBITS; b++) {
            t1[b] = MC_vec256_unpack_high(in[k][b], in[k + 1][b]);
        }

        MC_vec256_ama_asm(t0, t1, consts[1]);

        for (b = 0; b < GFBITS; b++) {
            in[k][b] = MC_vec256_unpack_low(t0[b], t1[b]);
        }
        for (b = 0; b < GFBITS; b++) {
            in[k + 1][b] = MC_vec256_unpack_high(t0[b], t1[b]);
        }

        for (b = 0; b < GFBITS; b++) {
            t0[b] = MC_vec256_unpack_low_2x(in[k][b], in[k + 1][b]);
        }
        for (b = 0; b < GFBITS; b++) {
            t1[b] = MC_vec256_unpack_high_2x(in[k][b], in[k + 1][b]);
        }

        MC_vec256_ama_asm(t0, t1, consts[0]);

        for (b = 0; b < GFBITS; b++) {
            in[k + 0][b] = MC_vec256_unpack_low_2x(t0[b], t1[b]);
        }
        for (b = 0; b < GFBITS; b++) {
            in[k + 1][b] = MC_vec256_unpack_high_2x(t0[b], t1[b]);
        }
    }


    // boradcast

    for (i = 0; i < GFBITS; i += 2) {
        // transpose

        for (k = 0; k < 32; k++) {
            if (i != GFBITS - 1) {
                buf.v[ reversal[2 * k + 0] ][1] = MC_vec256_extract2x(in[ k ][i + 1], 0);
                buf.v[ reversal[2 * k + 1] ][1] = MC_vec256_extract2x(in[ k ][i + 1], 1);
            }

            buf.v[ reversal[2 * k + 0] ][0] = MC_vec256_extract2x(in[ k ][i + 0], 0);
            buf.v[ reversal[2 * k + 1] ][0] = MC_vec256_extract2x(in[ k ][i + 0], 1);
        }

        MC_transpose_64x256_sp(buf.V);

        //

        pre.V[0][i / 2] = buf.V[32];
        buf.V[33] = MC_vec256_xor(buf.V[33], buf.V[32]);
        pre.V[1][i / 2] = buf.V[33];
        buf.V[35] = MC_vec256_xor(buf.V[35], buf.V[33]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[35]);
        buf.V[34] = MC_vec256_xor(buf.V[34], buf.V[35]);
        pre.V[2][i / 2] = buf.V[34];
        buf.V[38] = MC_vec256_xor(buf.V[38], buf.V[34]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[38]);
        buf.V[39] = MC_vec256_xor(buf.V[39], buf.V[38]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[39]);
        buf.V[37] = MC_vec256_xor(buf.V[37], buf.V[39]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[37]);
        buf.V[36] = MC_vec256_xor(buf.V[36], buf.V[37]);
        pre.V[3][i / 2] = buf.V[36];
        buf.V[44] = MC_vec256_xor(buf.V[44], buf.V[36]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[44]);
        buf.V[45] = MC_vec256_xor(buf.V[45], buf.V[44]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[45]);
        buf.V[47] = MC_vec256_xor(buf.V[47], buf.V[45]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[47]);
        buf.V[46] = MC_vec256_xor(buf.V[46], buf.V[47]);
        pre.V[2][i / 2] = MC_vec256_xor(pre.V[2][i / 2], buf.V[46]);
        buf.V[42] = MC_vec256_xor(buf.V[42], buf.V[46]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[42]);
        buf.V[43] = MC_vec256_xor(buf.V[43], buf.V[42]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[43]);
        buf.V[41] = MC_vec256_xor(buf.V[41], buf.V[43]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[41]);
        buf.V[40] = MC_vec256_xor(buf.V[40], buf.V[41]);
        pre.V[4][i / 2] = buf.V[40];
        buf.V[56] = MC_vec256_xor(buf.V[56], buf.V[40]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[56]);
        buf.V[57] = MC_vec256_xor(buf.V[57], buf.V[56]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[57]);
        buf.V[59] = MC_vec256_xor(buf.V[59], buf.V[57]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[59]);
        buf.V[58] = MC_vec256_xor(buf.V[58], buf.V[59]);
        pre.V[2][i / 2] = MC_vec256_xor(pre.V[2][i / 2], buf.V[58]);
        buf.V[62] = MC_vec256_xor(buf.V[62], buf.V[58]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[62]);
        buf.V[63] = MC_vec256_xor(buf.V[63], buf.V[62]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[63]);
        buf.V[61] = MC_vec256_xor(buf.V[61], buf.V[63]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[61]);
        buf.V[60] = MC_vec256_xor(buf.V[60], buf.V[61]);
        pre.V[3][i / 2] = MC_vec256_xor(pre.V[3][i / 2], buf.V[60]);
        buf.V[52] = MC_vec256_xor(buf.V[52], buf.V[60]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[52]);
        buf.V[53] = MC_vec256_xor(buf.V[53], buf.V[52]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[53]);
        buf.V[55] = MC_vec256_xor(buf.V[55], buf.V[53]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[55]);
        buf.V[54] = MC_vec256_xor(buf.V[54], buf.V[55]);
        pre.V[2][i / 2] = MC_vec256_xor(pre.V[2][i / 2], buf.V[54]);
        buf.V[50] = MC_vec256_xor(buf.V[50], buf.V[54]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[50]);
        buf.V[51] = MC_vec256_xor(buf.V[51], buf.V[50]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[51]);
        buf.V[49] = MC_vec256_xor(buf.V[49], buf.V[51]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[49]);
        buf.V[48] = MC_vec256_xor(buf.V[48], buf.V[49]);
        pre.V[5][i / 2] = buf.V[48];
        buf.V[16] = MC_vec256_xor(buf.V[16], buf.V[48]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[16]);
        buf.V[17] = MC_vec256_xor(buf.V[17], buf.V[16]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[17]);
        buf.V[19] = MC_vec256_xor(buf.V[19], buf.V[17]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[19]);
        buf.V[18] = MC_vec256_xor(buf.V[18], buf.V[19]);
        pre.V[2][i / 2] = MC_vec256_xor(pre.V[2][i / 2], buf.V[18]);
        buf.V[22] = MC_vec256_xor(buf.V[22], buf.V[18]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[22]);
        buf.V[23] = MC_vec256_xor(buf.V[23], buf.V[22]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[23]);
        buf.V[21] = MC_vec256_xor(buf.V[21], buf.V[23]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[21]);
        buf.V[20] = MC_vec256_xor(buf.V[20], buf.V[21]);
        pre.V[3][i / 2] = MC_vec256_xor(pre.V[3][i / 2], buf.V[20]);
        buf.V[28] = MC_vec256_xor(buf.V[28], buf.V[20]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[28]);
        buf.V[29] = MC_vec256_xor(buf.V[29], buf.V[28]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[29]);
        buf.V[31] = MC_vec256_xor(buf.V[31], buf.V[29]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[31]);
        buf.V[30] = MC_vec256_xor(buf.V[30], buf.V[31]);
        pre.V[2][i / 2] = MC_vec256_xor(pre.V[2][i / 2], buf.V[30]);
        buf.V[26] = MC_vec256_xor(buf.V[26], buf.V[30]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[26]);
        buf.V[27] = MC_vec256_xor(buf.V[27], buf.V[26]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[27]);
        buf.V[25] = MC_vec256_xor(buf.V[25], buf.V[27]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[25]);
        buf.V[24] = MC_vec256_xor(buf.V[24], buf.V[25]);
        pre.V[4][i / 2] = MC_vec256_xor(pre.V[4][i / 2], buf.V[24]);
        buf.V[8] = MC_vec256_xor(buf.V[8], buf.V[24]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[8]);
        buf.V[9] = MC_vec256_xor(buf.V[9], buf.V[8]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[9]);
        buf.V[11] = MC_vec256_xor(buf.V[11], buf.V[9]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[11]);
        buf.V[10] = MC_vec256_xor(buf.V[10], buf.V[11]);
        pre.V[2][i / 2] = MC_vec256_xor(pre.V[2][i / 2], buf.V[10]);
        buf.V[14] = MC_vec256_xor(buf.V[14], buf.V[10]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[14]);
        buf.V[15] = MC_vec256_xor(buf.V[15], buf.V[14]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[15]);
        buf.V[13] = MC_vec256_xor(buf.V[13], buf.V[15]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[13]);
        buf.V[12] = MC_vec256_xor(buf.V[12], buf.V[13]);
        pre.V[3][i / 2] = MC_vec256_xor(pre.V[3][i / 2], buf.V[12]);
        buf.V[4] = MC_vec256_xor(buf.V[4], buf.V[12]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[4]);
        buf.V[5] = MC_vec256_xor(buf.V[5], buf.V[4]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[5]);
        buf.V[7] = MC_vec256_xor(buf.V[7], buf.V[5]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[7]);
        buf.V[6] = MC_vec256_xor(buf.V[6], buf.V[7]);
        pre.V[2][i / 2] = MC_vec256_xor(pre.V[2][i / 2], buf.V[6]);
        buf.V[2] = MC_vec256_xor(buf.V[2], buf.V[6]);
        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[2]);
        buf.V[3] = MC_vec256_xor(buf.V[3], buf.V[2]);
        pre.V[1][i / 2] = MC_vec256_xor(pre.V[1][i / 2], buf.V[3]);
        buf.V[1] = MC_vec256_xor(buf.V[1], buf.V[3]);

        pre.V[0][i / 2] = MC_vec256_xor(pre.V[0][i / 2], buf.V[1]);
        t = MC_vec256_xor(buf.V[0], buf.V[1]);

        if (i != GFBITS - 1) {
            out128[i + 1][0] = MC_vec256_extract2x(t, 1);
        }
        out128[i + 0][0] = MC_vec256_extract2x(t, 0);

    }

    //

    for (j = 0; j < GFBITS; j++) {
        tmp[j] = MC_vec128_setbits((beta[0] >> j) & 1);
    }

    MC_vec128_mul(tmp, pre.v[0], tmp);

    for (b = 0; b < GFBITS; b++) {
        out128[b][1] = tmp[b];
    }

    for (i = 1; i < 6; i++) {
        for (j = 0; j < GFBITS; j++) {
            tmp[j] = MC_vec128_setbits((beta[i] >> j) & 1);
        }

        MC_vec128_mul(tmp, pre.v[i], tmp);

        for (b = 0; b < GFBITS; b++) {
            out128[b][1] = MC_vec128_xor(out128[b][1], tmp[b]);
        }
    }

    for (b = 0; b < GFBITS; b++) {
        v[0] = MC_vec128_extract(out128[b][0], 0);
        v[1] = MC_vec128_extract(out128[b][0], 1);
        v[2] = MC_vec128_extract(out128[b][1], 0);
        v[3] = MC_vec128_extract(out128[b][1], 1);

        out[b] = MC_vec256_set4x(v[0], v[1], v[2], v[3]);
    }
}

void MC_fft_tr(vec256 *out, vec256 in[][ GFBITS ]) {
    butterflies_tr(out, in);
    radix_conversions_tr(out);
}


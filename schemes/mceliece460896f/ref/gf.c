/*
  This file is for functions for field arithmetic
*/

#include "gf.h"

#include "params.h"

gf MC_gf_iszero(gf a) {
    uint32_t t = a;

    t -= 1;
    t >>= 19;

    return (gf) t;
}

gf MC_gf_add(gf in0, gf in1) {
    return in0 ^ in1;
}

gf MC_gf_mul(gf in0, gf in1) {
    int i;

    uint64_t tmp;
    uint64_t t0;
    uint64_t t1;
    uint64_t t;

    t0 = in0;
    t1 = in1;

    tmp = t0 * (t1 & 1);

    for (i = 1; i < GFBITS; i++) {
        tmp ^= (t0 * (t1 & (1 << i)));
    }

    //

    t = tmp & 0x1FF0000;
    tmp ^= (t >> 9) ^ (t >> 10) ^ (t >> 12) ^ (t >> 13);

    t = tmp & 0x000E000;
    tmp ^= (t >> 9) ^ (t >> 10) ^ (t >> 12) ^ (t >> 13);

    return tmp & GFMASK;
}

/* input: field element in */
/* return: (in^2)^2 */
static inline gf gf_sq2(gf in) {
    int i;

    const uint64_t B[] = {0x1111111111111111,
                          0x0303030303030303,
                          0x000F000F000F000F,
                          0x000000FF000000FF
                         };

    const uint64_t M[] = {0x0001FF0000000000,
                          0x000000FF80000000,
                          0x000000007FC00000,
                          0x00000000003FE000
                         };

    uint64_t x = in;
    uint64_t t;

    x = (x | (x << 24)) & B[3];
    x = (x | (x << 12)) & B[2];
    x = (x | (x << 6)) & B[1];
    x = (x | (x << 3)) & B[0];

    for (i = 0; i < 4; i++) {
        t = x & M[i];
        x ^= (t >> 9) ^ (t >> 10) ^ (t >> 12) ^ (t >> 13);
    }

    return x & GFMASK;
}

/* input: field element in, m */
/* return: (in^2)*m */
static inline gf gf_sqmul(gf in, gf m) {
    int i;

    uint64_t x;
    uint64_t t0;
    uint64_t t1;
    uint64_t t;

    const uint64_t M[] = {0x0000001FF0000000,
                          0x000000000FF80000,
                          0x000000000007E000
                         };

    t0 = in;
    t1 = m;

    x = (t1 << 6) * (t0 & (1 << 6));

    t0 ^= (t0 << 7);

    x ^= (t1 * (t0 & (0x04001)));
    x ^= (t1 * (t0 & (0x08002))) << 1;
    x ^= (t1 * (t0 & (0x10004))) << 2;
    x ^= (t1 * (t0 & (0x20008))) << 3;
    x ^= (t1 * (t0 & (0x40010))) << 4;
    x ^= (t1 * (t0 & (0x80020))) << 5;

    for (i = 0; i < 3; i++) {
        t = x & M[i];
        x ^= (t >> 9) ^ (t >> 10) ^ (t >> 12) ^ (t >> 13);
    }

    return x & GFMASK;
}

/* input: field element in, m */
/* return: ((in^2)^2)*m */
static inline gf gf_sq2mul(gf in, gf m) {
    int i;

    uint64_t x;
    uint64_t t0;
    uint64_t t1;
    uint64_t t;

    const uint64_t M[] = {0x1FF0000000000000,
                          0x000FF80000000000,
                          0x000007FC00000000,
                          0x00000003FE000000,
                          0x0000000001FE0000,
                          0x000000000001E000
                         };

    t0 = in;
    t1 = m;

    x = (t1 << 18) * (t0 & (1 << 6));

    t0 ^= (t0 << 21);

    x ^= (t1 * (t0 & (0x010000001)));
    x ^= (t1 * (t0 & (0x020000002))) << 3;
    x ^= (t1 * (t0 & (0x040000004))) << 6;
    x ^= (t1 * (t0 & (0x080000008))) << 9;
    x ^= (t1 * (t0 & (0x100000010))) << 12;
    x ^= (t1 * (t0 & (0x200000020))) << 15;

    for (i = 0; i < 6; i++) {
        t = x & M[i];
        x ^= (t >> 9) ^ (t >> 10) ^ (t >> 12) ^ (t >> 13);
    }

    return x & GFMASK;
}

/* input: field element den, num */
/* return: (num/den) */
gf MC_gf_frac(gf den, gf num) {
    gf tmp_11;
    gf tmp_1111;
    gf out;

    tmp_11 = gf_sqmul(den, den); // ^11
    tmp_1111 = gf_sq2mul(tmp_11, tmp_11); // ^1111
    out = gf_sq2(tmp_1111);
    out = gf_sq2mul(out, tmp_1111); // ^11111111
    out = gf_sq2(out);
    out = gf_sq2mul(out, tmp_1111); // ^111111111111

    return gf_sqmul(out, num); // ^1111111111110 = ^-1
}

gf MC_gf_inv(gf in) {
    return MC_gf_frac(in, ((gf) 1));
}

/* input: in0, in1 in GF((2^m)^t)*/
/* output: out = in0*in1 */
void MC_GF_mul(gf *out, const gf *in0, const gf *in1) {
    int i, j;

    gf prod[ SYS_T * 2 - 1 ];

    for (i = 0; i < SYS_T * 2 - 1; i++) {
        prod[i] = 0;
    }

    for (i = 0; i < SYS_T; i++)
        for (j = 0; j < SYS_T; j++) {
            prod[i + j] ^= MC_gf_mul(in0[i], in1[j]);
        }

    //

    for (i = (SYS_T - 1) * 2; i >= SYS_T; i--) {
        prod[i - SYS_T + 11] ^= MC_gf_mul(prod[i], (gf) 714);
        prod[i - SYS_T +  5] ^= MC_gf_mul(prod[i], (gf) 5296);
        prod[i - SYS_T +  4] ^= MC_gf_mul(prod[i], (gf) 728);
        prod[i - SYS_T +  0] ^= MC_gf_mul(prod[i], (gf) 5881);
    }

    for (i = 0; i < SYS_T; i++) {
        out[i] = prod[i];
    }
}


/*
  This file is for evaluating a polynomial at one or more field elements
*/
#include "root.h"

#include "params.h"

/* input: polynomial f and field element a */
/* return f(a) */
gf MC_eval(gf *f, gf a) {
    int i;
    gf r;

    r = f[ SYS_T ];

    for (i = SYS_T - 1; i >= 0; i--) {
        r = MC_gf_mul(r, a);
        r = MC_gf_add(r, f[i]);
    }

    return r;
}

/* input: polynomial f and list of field elements L */
/* output: out = [ f(a) for a in L ] */
void MC_root(gf *out, gf *f, gf *L) {
    int i;

    for (i = 0; i < SYS_N; i++) {
        out[i] = MC_eval(f, L[i]);
    }
}


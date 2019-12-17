#ifndef MC_BENES_H
#define MC_BENES_H
/*
  This file is for Benes network related functions
*/

#include "gf.h"

void MC_apply_benes(uint8_t *r, const uint8_t *bits, int rev);
void MC_support_gen(gf *s, const uint8_t *c);

#endif


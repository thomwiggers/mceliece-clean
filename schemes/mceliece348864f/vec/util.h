#ifndef MC_UTIL_H
#define MC_UTIL_H
/*
  This file is for loading/storing data in a little-endian fashion
*/


#include "gf.h"

#include <stdint.h>

void MC_store_i(unsigned char *out, uint64_t in, int i);

void MC_store2(unsigned char *dest, gf a);

uint16_t MC_load2(const unsigned char *src);

uint32_t MC_load4(const unsigned char *src);


void MC_irr_load(uint64_t *out, const unsigned char *in);

void MC_store8(unsigned char *out, uint64_t in);


uint64_t MC_load8(const unsigned char *in);

gf MC_bitrev(gf a);

#endif

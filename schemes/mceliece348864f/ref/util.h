/*
  This file is for loading/storing data in a little-endian fashion
*/

#ifndef UTIL_H
#define UTIL_H

#include "gf.h"
#include <stdint.h>

void MC_store2(unsigned char *, gf);
uint16_t MC_load2(const unsigned char *);

uint32_t MC_load4(const unsigned char *);

void MC_store8(unsigned char *, uint64_t );
uint64_t MC_load8(const unsigned char *);

gf MC_bitrev(gf);

#endif


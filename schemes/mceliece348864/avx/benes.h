#ifndef MC_BENES_H
#define MC_BENES_H
/*
  This file is for Benes network related functions
*/


#include "gf.h"
#include "vec128.h"

void MC_load_bits(uint64_t [][32], const unsigned char *);
void MC_benes(uint64_t *, uint64_t [][32], int);

#endif


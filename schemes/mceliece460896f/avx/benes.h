#ifndef MC_BENES_H
#define MC_BENES_H
/*
  This file is for Benes network related functions
*/


#include "vec128.h"

void MC_load_bits(vec128 [][32], const unsigned char *);
void MC_benes(vec128 *, vec128 [][32], int);

#endif


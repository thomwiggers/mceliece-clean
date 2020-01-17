/*
  This file is for Benes network related functions
*/

#ifndef BENES_H
#define BENES_H

#include "vec128.h"

void MC_load_bits(vec128 [][32], const unsigned char *);
void MC_benes(vec128 *, vec128 [][32], int);

#endif


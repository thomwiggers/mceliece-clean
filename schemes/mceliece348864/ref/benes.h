#ifndef MC_BENES_H
#define MC_BENES_H
/*
  This file is for Benes network related functions
*/


#include "gf.h"

void MC_apply_benes(unsigned char *, const unsigned char *, int);
void MC_support_gen(gf *, const unsigned char *);

#endif


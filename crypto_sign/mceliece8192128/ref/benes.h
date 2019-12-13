/*
  This file is for Benes network related functions
*/

#ifndef BENES_H
#define BENES_H

#include "gf.h"

void MC_apply_benes(unsigned char *, const unsigned char *, int);
void MC_support_gen(gf *, const unsigned char *);

#endif


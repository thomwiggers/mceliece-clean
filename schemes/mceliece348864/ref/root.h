#ifndef MC_ROOT_H
#define MC_ROOT_H
/*
  This file is for evaluating a polynomial at one or more field elements
*/


#include "gf.h"

gf MC_eval(gf *, gf);
void MC_root(gf *, gf *, gf *);

#endif


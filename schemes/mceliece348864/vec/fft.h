#ifndef MC_FFT_H
#define MC_FFT_H
/*
  This file is for the Gao-Mateer FFT
  sse http://www.math.clemson.edu/~sgao/papers/GM10.pdf
*/

#include <stdint.h>
#include "params.h"
#include "vec.h"

void MC_fft(vec [][ GFBITS ], vec *);

#endif


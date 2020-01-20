#ifndef FFT_H
#define FFT_H

/*
  This file is for the Gao-Mateer FFT
  sse http://www.math.clemson.edu/~sgao/papers/GM10.pdf
*/

#include <stdint.h>

#include "params.h"
#include "vec128.h"
#include "vec256.h"

void MC_fft(vec256 [][GFBITS], uint64_t *);

#endif


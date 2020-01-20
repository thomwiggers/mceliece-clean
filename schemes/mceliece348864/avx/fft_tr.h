#ifndef FFT_TR_H
#define FFT_TR_H
/*
  This file is for transpose of the Gao-Mateer FFT
*/


#include "params.h"
#include "vec256.h"

void MC_fft_tr(vec128 *, vec256 [][ GFBITS ]);

#endif


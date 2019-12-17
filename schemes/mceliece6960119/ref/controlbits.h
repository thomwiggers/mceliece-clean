#ifndef MC_CONTROLBITS_H
#define MC_CONTROLBITS_H
/*
  This file is for functions required for generating the control bits of the Benes network w.r.t. a random permutation
  see the Lev-Pippenger-Valiant paper https://www.computer.org/csdl/trans/tc/1981/02/06312171.pdf
*/


#include <stdint.h>

void MC_sort_63b(int, uint64_t []);
void MC_controlbits(unsigned char *, uint32_t *);

#endif


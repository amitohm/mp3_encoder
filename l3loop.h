#ifndef L3LOOP_H
#define L3LOOP_H

#include "layer3.h"

void huffman_init();

void iteration_loop(int *mdct_freq,short *mdct_enc,gr_info *side_info,int mean_bits);

#endif
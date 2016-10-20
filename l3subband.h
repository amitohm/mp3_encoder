#ifndef SUBBAND_H
#define SUBBAND_H

#include "layer3.h"

void subband_initialise();

void window_subband(short *buffer,int *sb_sample);

#endif

#ifndef _BITSTREAM_H
#define _BITSTREAM_H

#include "layer3.h"

#define         MIN(A, B)       ((A) < (B) ? (A) : (B))
#define         MAX(A, B)       ((A) > (B) ? (A) : (B))

void writeheader(unsigned int val,int len);

void putbits(unsigned int val,int len);

void encodeSideInfo(gr_info *cod_info);

void encodeMainData(char *sign_ptr,short *enc,gr_info *cod_info);

#endif
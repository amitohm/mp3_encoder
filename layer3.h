#ifndef LAYER3_H
#define LAYER3_H

#include <stdlib.h>
#include <inttypes.h>

/* Side information */
typedef struct {
        unsigned int part2_3_length;
        unsigned int big_values;
        unsigned int count1;
        unsigned int global_gain;
        unsigned int table_select0;
		unsigned int table_select1;
		unsigned int table_select2;
        unsigned int region0_count;
        unsigned int region1_count;
        unsigned int count1table_select;
        unsigned int address1;
        unsigned int address2;
        unsigned int address3;
		unsigned int main_data_end;
		unsigned int max_nonzero_coeff;
} gr_info;

void set_defaults();

void huffman_init();

void layer3_init();

void encoder(short *buffer_l,short *buffer_r);

#endif

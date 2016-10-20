#include <math.h>

#include "types.h"
#include "l3bitstream.h"
#include "l3loop.h"
#include "huffman.h"

extern const unsigned char low_gain_factor[];

extern const unsigned int largetbl[16*16];

extern const unsigned int table23[3*3];

extern const unsigned int table56[4*4];

extern struct huffcodetab ht[HTN];/* global memory block
									 array of all huffcodtable headers
									 0..31 Huffman code table 0..31
									 32,33 count1-tables */

extern char bv_scf[];
extern unsigned char t32l[],t33l[];

extern int sizeofhuffcodetab;

extern int mdct_freq[];
extern char mdct_sign[];
extern int mdct_limits[];
extern int prev_global_gain[];

extern int bin_count;



struct
{
    unsigned int region0_count;
    unsigned int region1_count;
} subdv_table[23] =
{
	{0, 0}, /* 0 bands */
	{0, 0}, /* 1 bands */
	{0, 0}, /* 2 bands */
	{0, 0}, /* 3 bands */
	{0, 0}, /* 4 bands */
	{0, 1}, /* 5 bands */
	{1, 1}, /* 6 bands */
	{1, 1}, /* 7 bands */
	{1, 2}, /* 8 bands */
	{2, 2}, /* 9 bands */
	{2, 3}, /* 10 bands */
	{2, 3}, /* 11 bands */
	{3, 4}, /* 12 bands */
	{3, 4}, /* 13 bands */
	{3, 4}, /* 14 bands */
	{4, 5}, /* 15 bands */
	{4, 5}, /* 16 bands */
	{4, 6}, /* 17 bands */
	{5, 6}, /* 18 bands */
	{5, 6}, /* 19 bands */
	{5, 7}, /* 20 bands */
	{6, 7}, /* 21 bands */
	{6, 7}, /* 22 bands */
};

extern struct scalefac_struct sfBandIndex[6];

extern int *scalefac_band_long;


//----------------------------------------------------------------------------------------------

int init_xrpow(int *mdct_freq_ptr,char *sign_ptr,gr_info *cod_info)
{
	unsigned int i;
	int temp,sum;
	unsigned char gain;
	
	sum = 0;
	
	i = 440;	//441 to 575 have been set to 0.see mdct code.
	while(mdct_freq_ptr[i]==0)
	{
		i--;
	}
	
	i++;
	i >>= 1;
	i <<= 1;
	
	cod_info->max_nonzero_coeff = i;
	
	for(i=0;i<=cod_info->max_nonzero_coeff;i++)
	{
		gain = 0;
		
		temp = mdct_freq_ptr[i];
		
		sign_ptr[i] = 0;
		if(temp<0)
			sign_ptr[i] = 1;
		
		temp = abs(temp);
		
		sum += temp;
		
		if(temp<=160)
		{
			gain = MIN(temp,100);
			gain = low_gain_factor[gain];
		}
		else
		{
			gain = 9;
			
			while(temp>mdct_limits[gain-1])
			{
				gain--;
			}
		}
		
		mdct_freq_ptr[i] = temp * gain;
	}
	
	return sum;
}

//----------------------------------------------------------------------------------------------

int ix_max(short *ix,short *end)
{
    int max1 = 0, max2 = 0;
	
    do
	{
		int x1 = *ix++;
		int x2 = *ix++;
		
		if(max1 < x1) 
			max1 = x1;
		
		if(max2 < x2) 
			max2 = x2;
		
    }while (ix < end);
	
    if(max1 < max2) 
		max1 = max2;
	
    return max1;
}

//----------------------------------------------------------------------------------------------

int count_bit_ESC(short *ix,short *end,int t1,int t2,int *s)
{
    /* ESC-table is used */
    int linbits = ht[t1].xlen * 65536 + ht[t2].xlen;
    int sum = 0, sum2;
	
    do
	{
		int x = *ix++;
		int y = *ix++;
		
		x = MIN(x,15);
		if(x==15)
		{
			sum += linbits;
		}
		x *= 16;
		
		y = MIN(y,15);
		if(y > 14)
		{
			sum += linbits;
		}
		x += y;
		
		sum += largetbl[x];
    }while(ix < end);
	
    sum2 = sum & 0xFFFF;
    sum >>= 16;
	
    if(sum > sum2)
	{
		sum = sum2;
		t1 = t2;
    }
	
    *s += sum;
    return t1;
}

//----------------------------------------------------------------------------------------------

int count_bit_noESC(short *ix,short *end,int *s)
{
    /* No ESC-words */
    int	sum1 = 0;
    const char *hlen1 = ht[1].hlen;

    do
	{
		int x = ix[0] * 2 + ix[1];
		ix += 2;
		sum1 += hlen1[x];
    } while (ix < end);
	
    *s += sum1;

    return 1;
}

//----------------------------------------------------------------------------------------------

int count_bit_noESC_from2(short *ix,short *end,int t1,int *s)
{
    /* No ESC-words */
    unsigned int sum = 0, sum2;
    const int xlen = ht[t1].xlen;
    const unsigned int *hlen;

    if(t1 == 2)
		hlen = table23;
    else
		hlen = table56;

    do
	{
		int x = ix[0] * xlen + ix[1];
		ix += 2;
		sum += hlen[x];
    }while(ix < end);

    sum2 = sum & 0xffff;
    sum >>= 16;

    if(sum > sum2)
	{
		sum = sum2;
		t1++;
    }

    *s += sum;

    return t1;
}

//----------------------------------------------------------------------------------------------

int count_bit_noESC_from3(short *ix,short *end,int t1,int *s)
{
    /* No ESC-words */
    int	sum1 = 0;
    int	sum2 = 0;
    int	sum3 = 0;
    const int xlen = ht[t1].xlen;
    const char *hlen1 = ht[t1].hlen;
    const char *hlen2 = ht[t1+1].hlen;
    const char *hlen3 = ht[t1+2].hlen;
    int t;

    do
	{
		int x = ix[0] * xlen + ix[1];
		ix += 2;
		sum1 += hlen1[x];
		sum2 += hlen2[x];
		sum3 += hlen3[x];
    }while(ix < end);

    t = t1;

    if(sum1 > sum2)
	{
		sum1 = sum2;
		t++;
    }

    if(sum1 > sum3)
	{
		sum1 = sum3;
		t = t1+2;
    }

    *s += sum1;

    return t;
}


//----------------------------------------------------------------------------------------------

int choose_table(short *ix,short *end,int *bits)
{
    int max;
    int choice, choice2;
    int huf_tbl_noESC[] = {1,2,5,7,7,10,10,13,13,13,13,13,13,13,13};
	
    max = ix_max(ix, end);
	
    switch(max)
	{
    case 0:
		return max;
		
    case 1:
		return count_bit_noESC(ix,end,bits);
		
    case 2:
    case 3:
		return count_bit_noESC_from2(ix,end,huf_tbl_noESC[max - 1],bits);
		
    case 4: case 5: case 6:
    case 7: case 8: case 9:
    case 10: case 11: case 12:
    case 13: case 14: case 15:
		return count_bit_noESC_from3(ix,end,huf_tbl_noESC[max - 1],bits);
		
    default:
		max -= 15;
		
		for(choice2=24;choice2<32;choice2++)
		{
			if(ht[choice2].linmax >= max)
			{
				break;
			}
		}
		
		for(choice = choice2 - 8; choice < 24; choice++)
		{
			if(ht[choice].linmax >= max)
			{
				break;
			}
		}
		
		return count_bit_ESC(ix,end,choice,choice2,bits);
    }
}


//----------------------------------------------------------------------------------------------


int noquant_count_bits(short *ix,gr_info *cod_info)
{
	int i;
	int a0,a1,bits;
	
	i = cod_info->max_nonzero_coeff;
	
	for(; i > 1; i -= 2)
	{
	    if(ix[i - 1] | ix[i - 2])
	        break;
	}
	cod_info->count1 = i;
	
	a0 = a1 = 0;
	for(; i > 3; i -= 4)
	{
	    int p;
	    /* hack to check if all values <= 1 */
	    if ((unsigned int)(ix[i-1] | ix[i-2] | ix[i-3] | ix[i-4]) > 1)
	        break;
		
	    p = ((ix[i-4] * 2 + ix[i-3]) * 2 + ix[i-2]) * 2 + ix[i-1];
	    a0 += t32l[p];
	    a1 += t33l[p];
    }
	
	bits = a0;
    cod_info->count1table_select = 0;
    if(a0 > a1)
	{
		bits = a1;
		cod_info->count1table_select = 1;
    }
	
    cod_info->big_values = i;
    if(i == 0)
	    return bits;
	
	a0 = cod_info->region0_count = bv_scf[i-2];
	a1 = cod_info->region1_count = bv_scf[i-1];
	
    a1 = scalefac_band_long[a0 + a1 + 2];
	a0 = scalefac_band_long[a0 + 1];

	if(a1 < i)
		cod_info->table_select2 = choose_table(ix + a1, ix + i, &bits);

	/* have to allow for the case when bigvalues < region0 < region1 */
    /* (and region0, region1 are ignored) */
    a0 = MIN(a0,i);
    a1 = MIN(a1,i);

	/* Count the number of bits necessary to code the bigvalues region. */
    if(0 < a0)
		cod_info->table_select0 = choose_table(ix, ix + a0, &bits);
	
    if(a0 < a1)
		cod_info->table_select1 = choose_table(ix + a0, ix + a1, &bits);

    return bits;
}

//----------------------------------------------------------------------------------------------

#define ROUNDFAC 425093
void quantize(int *xr,short *ix,int gain,gr_info *cod_info)
{
    int64_t int_x0;
    int l;
	
	l = cod_info->max_nonzero_coeff;
	
    do
	{
		int_x0 = (int64_t)gain * xr[l] + ROUNDFAC;
		
		ix[l] = (int)(int_x0>>20);
    }while(l--);
}

//----------------------------------------------------------------------------------------------

int count_bits(int *xr,short *ix,gr_info *cod_info)
{
	int ipow20[] = {440872,387141,339959,298527,262144,230195,202141,177505,155872,136875,120194,105545,92682,81386,71468,62757,55109,48393,42495,37316,32768,28774,25268,22188,19484,17109,15024,13193,11585,10173,8933,7845,6889,6049,5312,4664,4096,3597,3158,2774,2435,2139,1878,1649,1448,1272,1117,981,861,756,664,583,512,450,395,347,304,267,235,206,181,159,140,123,108,95,83,73,64,56,49,43,38,33,29,26,23,20,17,15,13,12,10,9,8,7,6,5,5,4,4,3,3,2,2,2,2,1,1,1,1,1,1,1,1,1};
	int gain,bits;

	gain = ipow20[cod_info->global_gain - 110];

	quantize(xr,ix,gain,cod_info);

	bits = noquant_count_bits(ix,cod_info);
	
	return bits;
}


//----------------------------------------------------------------------------------------------


void bin_search_StepSize(int *xr,short *ix,int desired_rate, gr_info *cod_info)
/***************************************************************************/
/* The code selects the best quantizerStepSize for a particular set        */
/* of scalefacs                                                            */
/***************************************************************************/
{
    char step,min_step,direction;
    int down_diff[5],up_diff[5];
    int min_diff;
    int bits=0,diff_bits;

    up_diff[4] = desired_rate<<1;
    up_diff[3] = desired_rate;
    up_diff[2] = (desired_rate*3)>>2;
    up_diff[1] = desired_rate>>1;
    up_diff[0] = 0;

    down_diff[4] = desired_rate - 20;
    down_diff[3] = desired_rate>>1;
    down_diff[2] = (desired_rate*3)>>3;
    down_diff[1] = desired_rate>>2;
    down_diff[0] = 0;

    min_step = 4;
    direction = 0;
    min_diff = desired_rate - 1;
    do {
	bin_count++;
	bits = count_bits(xr,ix,cod_info);
	if(bits>desired_rate) {
	    diff_bits = desired_rate - bits;
	    if(direction == -1) {
		min_step--;
	    } else {
		step = 4;
		diff_bits = bits - desired_rate;
		while(!(diff_bits>=up_diff[step]))
		    step--;
		min_step = MIN(step,min_step);
	    }
	    step = 1<<min_step;
	    direction = 1;
	} else {
	    diff_bits = desired_rate - bits;
	    if (direction == 1)	{
		min_step--;
	    } else {
		step = 4;
		while (!(diff_bits>=down_diff[step]))
		    step--;
		min_step = MIN(step,min_step);
	    }
	    if ((diff_bits == min_diff) || (diff_bits<down_diff[1]))
		break;

	    min_diff = MIN(min_diff,diff_bits);
	    step = -(1<<min_step);
	    direction = -1;
	}
	cod_info->global_gain += step;		
    } while(bits!=desired_rate);

    cod_info->part2_3_length = bits;
}

//----------------------------------------------------------------------------------------------

void iteration_loop(int *mdct_freq_ptr,short *mdct_enc,gr_info *side_info,int mean_bits) 
/************************************************************************/
/*					  iteration_loop()                                  */
/************************************************************************/
{
    gr_info *cod_info;
	int ResvSize = 0;
    int max_bits;
    int ch,gr;
	short *enc_ptr;
	char *sign_ptr;

	sign_ptr = mdct_sign;
	
    scalefac_band_long = &sfBandIndex[config.mpeg.samplerate_index + (config.mpeg.type * 3)].l[0];
	
	/* calculation of number of available bit( per granule ) */
	if(config.wave.channels == 2)
		max_bits = mean_bits>>1;
	else
		max_bits = mean_bits;
	
    for(gr=0;gr<config.mpeg.gr;gr++)
    {
		enc_ptr = mdct_enc + ((SAMP_PER_FRAME + DECDELAY)*gr);
		
        for(ch=0;ch<config.wave.channels;ch++)
        {
			ResvSize += max_bits;
			
			cod_info = &side_info[(gr<<1)+ch];
			
			cod_info->global_gain = prev_global_gain[ch];
			
			if(init_xrpow(mdct_freq_ptr,sign_ptr,cod_info))
			{
				bin_search_StepSize(mdct_freq_ptr,enc_ptr,ResvSize,cod_info);
			}
			
			ResvSize -= cod_info->part2_3_length;
			
			prev_global_gain[ch] = cod_info->global_gain;
			
			mdct_freq_ptr += SAMP_PER_FRAME_2;
			enc_ptr += SAMP_PER_FRAME_2;
			sign_ptr += SAMP_PER_FRAME_2;
			
        } /* for ch */
    } /* for gr */
	
	//side_info[0].part2_3_length += ResvSize;
}

//----------------------------------------------------------------------------------------------

void huffman_init()
{
    int i;
	
	scalefac_band_long = &sfBandIndex[config.mpeg.samplerate_index + (config.mpeg.type * 3)].l[0];

    for(i = 2; i <= 576; i += 2)
	{
		int scfb_anz = 0, index;
		
		while(scalefac_band_long[++scfb_anz] < i);
		
		index = subdv_table[scfb_anz].region0_count;
		
		while(scalefac_band_long[index + 1] > i)
			index--;
		
		if(index < 0)
		{
		/* this is an indication that everything is going to
		be encoded as region0:  bigvalues < region0 < region1
		so lets set region0, region1 to some value larger
			than bigvalues */
			index = subdv_table[scfb_anz].region0_count;
		}
		
		bv_scf[i-2] = index;
		
		index = subdv_table[scfb_anz].region1_count;
		
		while(scalefac_band_long[index + bv_scf[i-2] + 2] > i)
			index--;
		
		if(index < 0)
		{
			index = subdv_table[scfb_anz].region1_count;
		}
		
		bv_scf[i-1] = index;
    }
}


/**/

/*int bits;
    int CurrentStep = 4;
    int flag_GoneOver = 0;
    int Direction = 0;
	
    do
	{
		int step;

		bincount++;
        bits = count_bits(xr,ix,cod_info);
		
        if(CurrentStep == 1 || bits == desired_rate)
			break; // nothing to adjust anymore
		
        if(bits>desired_rate)
		{
            // increase Quantize_StepSize
            if(Direction == -1)
                flag_GoneOver = 1;
			
			if(flag_GoneOver)
				CurrentStep >>= 1;

            Direction = 1;
			step = CurrentStep;
        }
		else
		{
            // decrease Quantize_StepSize
            if(Direction == 1)
                flag_GoneOver = 1;
			
			if(flag_GoneOver) 
				CurrentStep >>= 1;

            Direction = -1;
			step = -CurrentStep;
        }
		cod_info->global_gain += step;

    }while((cod_info->global_gain < 256) && (cod_info->global_gain > 109));//to ensure global_gain is between 0 and 440872.
	
    if(cod_info->global_gain < 0)
	{
		cod_info->global_gain = 0;
		bits = count_bits(xr,ix,cod_info);
    }
	else if(cod_info->global_gain > 255)
	{
		cod_info->global_gain = 255;
		bits = count_bits(xr,ix,cod_info);
    }
	else if(bits > desired_rate)
	{
		cod_info->global_gain++;
		bits = count_bits(xr,ix,cod_info);
    }

    cod_info->part2_3_length = bits;*/


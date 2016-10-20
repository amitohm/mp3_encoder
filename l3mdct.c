#include <string.h>

#include "types.h"
#include "l3subband.h"
#include "l3mdct.h"

extern int order[];
extern const short win[NL],tantab_l[9],cx[8],cs[8],ca[8];
extern int mdct_freq[];
extern int sb_sample_l[],sb_sample_r[];

//-----------------------------------------------------------------

void mdct_long(int *out,int *in)
{
    int ct,st;
	
	int tc1, tc2, tc3, tc4, ts5, ts6, ts7, ts8;
	int ts1, ts2, ts3, ts4, tc5, tc6, tc7, tc8;
	
	/* 1,2, 5,6, 9,10, 13,14, 17 */
	tc1 = in[17] - in[ 9];
	tc3 = in[15] - in[11];
	tc4 = in[14] - in[12];
	ts5 = in[ 0] + in[ 8];
	ts6 = in[ 1] + in[ 7];
	ts7 = in[ 2] + in[ 6];
	ts8 = in[ 3] + in[ 5];
	
	out[17] = (ts5 + ts7 - ts8) - (ts6 - in[4]);
	st = ((ts5 + ts7- ts8)*cx[7])>>10;
	st += (ts6 - in[4]);
	ct = ((tc1 - tc3 - tc4)*cx[6])>>10;
	out[5] = ct + st;
	out[6] = ct - st;
	
	tc2 = ((in[16] - in[10])*cx[6])>>10;
	ts6 = (ts6*cx[7])>>10;
	ts6 += in[4];
	ct =  (tc1*cx[0] + tc3*cx[1] + tc4*cx[2])>>10;
	ct += tc2;
	st = (-ts5*cx[4] - ts7*cx[5] + ts8*cx[3])>>10;
	st += ts6;
	out[1] = ct + st;
	out[2] = ct - st;
	
	ct =  (tc1*cx[1] - tc3*cx[2] + tc4*cx[0])>>10;
	ct -= tc2;
	st = (-ts5*cx[5] - ts7*cx[3] + ts8*cx[4])>>10;
	st += ts6;
	out[ 9] = ct + st;
	out[10] = ct - st;
	
	ct = (tc1*cx[2] + tc3*cx[0] - tc4*cx[1])>>10;
	ct -= tc2;
	st = (ts5*cx[3] + ts7*cx[4] - ts8*cx[5])>>10;
	st -= ts6;
	out[13] = ct + st;
	out[14] = ct - st;
	
	ts1 = in[ 8] - in[ 0];
	ts3 = in[ 6] - in[ 2];
	ts4 = in[ 5] - in[ 3];
	tc5 = in[17] + in[ 9];
	tc6 = in[16] + in[10];
	tc7 = in[15] + in[11];
	tc8 = in[14] + in[12];
	
	out[0]  = (tc5 + tc7 + tc8) + (tc6 + in[13]);
	ct = ((tc5 + tc7 + tc8)*cx[7])>>10;
	ct -= (tc6 + in[13]);
	st = ((ts1 - ts3 + ts4)*cx[6])>>10;
	out[11] = ct + st;
	out[12] = ct - st;
	
	ts2 = ((in[7] - in[1])*cx[6])>>10;
	tc6 = (-tc6*cx[7])>>10;
	tc6 += in[13];
	ct = (tc5*cx[3] + tc7*cx[4] + tc8*cx[5])>>10;
	ct -= tc6;
	st = (ts1*cx[2] + ts3*cx[0] + ts4*cx[1])>>10;
	st += ts2;
	out[3] = ct + st;
	out[4] = ct - st;
	
	ct = (-tc5*cx[5] - tc7*cx[3] - tc8*cx[4])>>10;
	ct += tc6;
	st =  (ts1*cx[1] - ts3*cx[2] - ts4*cx[0])>>10;
	st += ts2;
	out[7] = ct + st;
	out[8] = ct - st;
	
	ct = (-tc5*cx[4] - tc7*cx[5] - tc8*cx[3])>>10;
	ct += tc6;
	st = (ts1*cx[0] + ts3*cx[1] - ts4*cx[2])>>10;
	st -= ts2;
	out[15] = ct + st;
	out[16] = ct - st;
}


//-----------------------------------------------------------------


void mdct_sub(short *buffer_l,short *buffer_r)
{
    int k,gr,ch;
	int *sb_sample_gr[2];
	int *sb_sample_ptr;
	int *mdct_freq_ptr;
	short *buffer_ptr;
	
    buffer_ptr = buffer_l + 286;
	
	sb_sample_gr[0] = sb_sample_l;
	sb_sample_gr[1] = sb_sample_l + SAMP_PER_FRAME_2;
	
    /* thinking cache performance, ch->gr loop is better than gr->ch loop */
    for(ch=0;ch<config.wave.channels;ch++)
	{
		for(gr=0;gr<config.mpeg.gr;gr++)
		{
			int	band;
			
			sb_sample_ptr = sb_sample_gr[1 - gr];

			mdct_freq_ptr = mdct_freq + (SAMP_PER_FRAME_2 * ((gr<<1)+ch));
			
			for(k=0;k<9/*18/2*/;k++)
			{
				window_subband(buffer_ptr,sb_sample_ptr);
				buffer_ptr += 32;
				sb_sample_ptr += 32;
				window_subband(buffer_ptr,sb_sample_ptr);
				buffer_ptr += 32;
				sb_sample_ptr += 32;
				
				//Compensate for inversion in the analysis filter
				for(band=1;band<32;band+=2)
				{
					sb_sample_ptr[band-32] *= -1;
				}
			}
			
			/* Perform imdct of 18 previous subband samples
			   + 18 current subband samples */
			for(band=0;band<24;band++)
			{
				int work[18];
				int *band0,*band1;
				
				band0 = sb_sample_gr[  gr] + order[band];
				band1 = sb_sample_gr[1-gr] + order[band];
				
				for(k=-NL/4;k<0;k++)
				{
					int a,b;
					
					a = (int)(win[k+27] * band1[(k+9)*32] + win[k+36] * band1[(8-k)*32])>>10;
					b = (int)(win[k+ 9] * band0[(k+9)*32] - win[k+18] * band0[(8-k)*32])>>10;
					
					work[k+ 9] = -(b*tantab_l[k+9])>>10;
					work[k+ 9] += a;
					
					work[k+18] = (a*tantab_l[k+9])>>10;
					work[k+18] += b;
				}
				mdct_long(mdct_freq_ptr, work);
				
				//Perform aliasing reduction butterfly
				if(band != 0)
				{
					for(k=7;k>=0;--k)
					{
						int bu,bd;
						
						bu = (int)(mdct_freq_ptr[k] * ca[k] + mdct_freq_ptr[-1-k] * cs[k])>>10;
						bd = (int)(mdct_freq_ptr[k] * cs[k] - mdct_freq_ptr[-1-k] * ca[k])>>10;
						
						mdct_freq_ptr[-1-k] = bu;
						mdct_freq_ptr[k] = bd;
					}
				}
				mdct_freq_ptr += 18;
			}
			
			memset(mdct_freq_ptr,0,18*sizeof(int));
			
			for(k=7;k>=0;--k)
			{
				int bu,bd;
				
				bu = (int)(mdct_freq_ptr[k] * ca[k] + mdct_freq_ptr[-1-k] * cs[k])>>10;
				bd = (int)(mdct_freq_ptr[k] * cs[k] - mdct_freq_ptr[-1-k] * ca[k])>>10;
				
				mdct_freq_ptr[-1-k] = bu;
				mdct_freq_ptr[k] = bd;
			}
			
			mdct_freq_ptr += 18;
			
			for(band=25;band<32;band++)
			{
				memset(mdct_freq_ptr,0,18*sizeof(int));
				
				mdct_freq_ptr += 18;
			}
		}
		
		buffer_ptr = buffer_r + 286;
		
		if(config.mpeg.gr == 1)
		{
			memcpy(sb_sample_gr[0], sb_sample_gr[1], 576 * sizeof(int));
		}
		
		sb_sample_gr[0] = sb_sample_r;
		sb_sample_gr[1] = sb_sample_r + SAMP_PER_FRAME_2;
    }
}
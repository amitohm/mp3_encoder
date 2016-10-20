#include <string.h>

#include "types.h"
#include "layer3.h"
#include "l3subband.h"
#include "l3mdct.h"
#include "l3loop.h"
#include "l3bitstream.h"

extern unsigned char bit_count,output;
extern int slot_lag;
extern int frac_slots_per_frame;
extern int whole_slots_per_frame;
extern int prev_quantizerStepSize[2];
extern int sb_sample_l[],sb_sample_r[];
extern gr_info side_info[4];
extern int mdct_freq[2304];
extern char mdct_sign[];


void set_defaults()
{
		config.mpeg.type = 1;	//MPEG-1
		config.mpeg.layr = 1;	//Layer - III
		config.mpeg.mode = 2;
		config.mpeg.gr	 = 2;	//granules = 2(MPEG-1)......granules = 1(MPEG-2)
		config.mpeg.bitr = 128;
/**/	config.mpeg.samplerate_index = 0;	//{44100,48000,32000}
/**/	config.mpeg.bitrate_index =	9;		//{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320}
		config.mpeg.emph = 0; 
		config.mpeg.crc  = 0;
		config.mpeg.ext  = 0;
		config.mpeg.mode_ext  = 0;
		config.mpeg.copyright = 0;
		config.mpeg.original  = 1;

	//WAVE
		config.wave.bits = 16;
/**/	config.wave.samplerate = 44100;
		config.wave.channels = 2;
		config.mpeg.samples_per_frame = SAMP_PER_FRAME;		//samplerate >= 32000
		//config.mpeg.samples_per_frame = SAMP_PER_FRAME_2;		//samplerate < 32000
}



void layer3_init()
{
	double avg_slots_per_frame;
	
	//sb_sample_initialize
	memset((int *)&sb_sample_l,0,1152*sizeof(int));
	memset((int *)&sb_sample_r,0,1152*sizeof(int));

	//mdct_sign initialize
	memset((int *)&mdct_sign,0,2304*sizeof(char));
	
	/* Figure average number of 'slots' per frame. */
	avg_slots_per_frame = ((double)config.mpeg.samples_per_frame*(config.mpeg.bitr>>3)*1000) / (double)config.wave.samplerate;
	
    whole_slots_per_frame = (int)avg_slots_per_frame;
    frac_slots_per_frame  = (int)((avg_slots_per_frame - (double)whole_slots_per_frame)*1000);
    slot_lag              = -frac_slots_per_frame;
}

void encoder(short *buffer_l,short *buffer_r)
{
    int             mean_bits;
    short *mdct_enc;
	
	if(frac_slots_per_frame)
	{
		if(slot_lag>(frac_slots_per_frame-1000))
		{ /* No padding for this frame */
			slot_lag -= frac_slots_per_frame;
			config.mpeg.padding = 0;
		}
		else
		{ /* Padding for this frame  */
			slot_lag += (1000-frac_slots_per_frame);
			config.mpeg.padding = 1;
		}
	}
	
	config.mpeg.bits_per_frame = (whole_slots_per_frame + config.mpeg.padding)<<3;
	
	output = 0;
	bit_count = 8;
	
	if(config.wave.channels==2)
		mean_bits = (config.mpeg.bits_per_frame - 288)>>1;		//for 2 channels
	else
		mean_bits = (config.mpeg.bits_per_frame - 168)>>1;		//for 1 channels
	
	/* apply mdct to the polyphase output */
	mdct_sub(buffer_l,buffer_r);

	mdct_enc = buffer_l; //buffer is reused to store quatized values.
	
	/* bit and noise allocation */
	iteration_loop(mdct_freq,mdct_enc,side_info,mean_bits);
	
	/* write the frame to the bitstream */
	encodeSideInfo(side_info);
	
	encodeMainData(mdct_sign,mdct_enc,side_info);
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "wave.h"

extern int Framecount;
extern short buffer_l[1680],buffer_r[1680];

int stereo_read_samples(short *buffer_l,short *buffer_r,int frame_size)
{
	int i;
	int samples_read = 0;

	for(i=0;i<DECDELAY;i++)
	{
		buffer_l[i] = buffer_l[SAMP_PER_FRAME+i];
		buffer_r[i] = buffer_r[SAMP_PER_FRAME+i];
	}
	
	buffer_l += DECDELAY;
	buffer_r += DECDELAY;
	if(config.inputsize)
	{
		Framecount++;

		for(i=0;i<SAMP_PER_FRAME;i++)
		{
			*buffer_l++ = *config.inbuf++;
			*buffer_r++ = *config.inbuf++;
			
			samples_read++;
			config.inputsize -= 2;
		}
	}

    return (samples_read<<1);
}

int mono_read_samples(short *buffer_l,short *buffer_r,int frame_size)
{
	int i;
	int samples_read = 0;
	
	buffer_l += DECDELAY;
	buffer_r += DECDELAY;
	
	for(i=0;i<SAMP_PER_FRAME;i++)
	{
		*buffer_l++ = *config.inbuf++;
		*buffer_r++ = 0;

		samples_read++;
	}

    return (samples_read);
}

/*if(config.mpeg.samples_per_frame==SAMP_PER_FRAME)
{
	for(i=0;i<SAMP_PER_FRAME;i++)
	{
		buffer0[i] = sample_buffer[2*i];
		buffer1[i] = sample_buffer[2*i+1];
	}
}
else
{
	// upsampling for 16kHz,22.05kHz,24kHz
	for(i=0;i<(SAMP_PER_FRAME-1);i++)
	{
		if(i%2==0)
		{
			buffer0[i] = sample_buffer[i];
			buffer1[i] = sample_buffer[i+1];
		}
		else
	{
		buffer0[i] = (sample_buffer[i-1]+sample_buffer[i+1])>>1;
		buffer1[i] = (sample_buffer[i]+sample_buffer[i+2])>>1;
	}
}
			
buffer0[i] = sample_buffer[i-1];
buffer1[i] = sample_buffer[i];*/

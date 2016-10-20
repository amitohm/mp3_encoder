#ifndef TYPES_H
#define TYPES_H


#define			SAMP_PER_FRAME			1152
#define			SAMP_PER_FRAME_2		576
#define			DECDELAY				528


#define			HAN_SIZE				1024
#define			SBLIMIT					32
#define			SBLIMIT_SHIFT			5


struct scalefac_struct
{
   int l[23];
   int s[14];
};

typedef struct {
    int channels;
    int bits;
    int	samplerate;
} wave_t;

typedef struct {
    int  type;
    int  layr;
    int  mode;
	int  gr;
    int  bitr;
    int  emph;
    int  padding;
    int  samples_per_frame;
    int  bits_per_frame;
    int  bitrate_index;
    int  samplerate_index;
    int  crc;
    int  ext;
    int  mode_ext;
    int  copyright;
    int  original;
} mpeg_t;

typedef struct {
	//input
	short *inbuf;	
	int inputsize;
    wave_t wave;

	//output
	unsigned char *outbuf;
    mpeg_t mpeg;
} config_t;
extern config_t config;

#endif
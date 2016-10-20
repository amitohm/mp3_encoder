#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "l3bitstream.h"
#include "wave.h"
#include "l3loop.h"
#include "layer3.h"


config_t config;

extern short buffer[];
extern int Framecount;
extern unsigned int outputsize;

void buffer_init(short *buffer_l,short *buffer_r)
{
	int i;

	buffer_l += SAMP_PER_FRAME;
	buffer_r += SAMP_PER_FRAME;
	
	for(i=0;i<DECDELAY;i++)
	{
		buffer_l[i] = 0;
		buffer_r[i] = 0;
	}
}

int main(int argc, char **argv)
{
    FILE *infile,*outfile;
    int filesize;
    short *buffer_l,*buffer_r;
    
    if (argc!=2) {
	printf("Incorrect arguments!!!\n");
	exit(-1);
    }
    if((infile = fopen(argv[1],"rb")) == NULL) {
	printf("Unable to open file %s\n", argv[1]);
	exit(-1);
    }
    fseek(infile,0,SEEK_END);
    filesize = ftell(infile);
    fseek(infile,0,SEEK_SET);

    config.inbuf = (short *)malloc((filesize>>1)*sizeof(short));
    if(config.inbuf == NULL) {
	printf("Error allocating memory.");
	exit(-1);
    }
    config.inputsize = fread(config.inbuf,sizeof(short),(filesize>>1),infile);
    fclose(infile);

    config.outbuf = (unsigned char *)malloc((1<<19)*sizeof(unsigned char));
    if(config.outbuf == NULL) {
	printf("Error allocating memory.");
	exit(-1);
    }

    outputsize = 0;
    printf("Start encoding.");
    set_defaults();
    buffer_l = buffer;
    buffer_r = buffer + SAMP_PER_FRAME + DECDELAY;
    buffer_init(buffer_l,buffer_r);
    layer3_init();
    huffman_init();
    while(stereo_read_samples(buffer_l,buffer_r,config.mpeg.samples_per_frame))	{
	encoder(buffer_l,buffer_r);
    }
    if((outfile = fopen(argv[2],"wb")) == NULL) {
	printf("Could not create \"%s\".\n",argv[2]);
	exit(-1);
    }
    fwrite(config.outbuf,sizeof(unsigned char),outputsize,outfile);
    fclose(outfile);
    printf("\nDone.");
    return 0;
}


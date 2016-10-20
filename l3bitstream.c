#include <stdlib.h>
#include <assert.h>

#include "l3bitstream.h" /* the public interface */
#include "huffman.h"
#include "types.h"

extern char mdct_sign[];
extern unsigned char bit_count,output;
extern unsigned int outputsize;
extern unsigned char *resv_buf;
extern int main_data_begin;

extern struct huffcodetab ht[HTN];/* global memory block		*/
				/* array of all huffcodtable headers	*/
				/* 0..31 Huffman code table 0..31	*/
				/* 32,33 count1-tables	*/

extern struct scalefac_struct sfBandIndex[6];
extern int *scalefac_band_long;


//----------------------------------------------------------------------------------------------

int Huffman_coder_count1(char *sign_ptr,short *ix,gr_info *cod_info)
{
    /* Write count1 area */
    const struct huffcodetab *h = &ht[cod_info->count1table_select + 32];
    int i,bits=0;

	sign_ptr += cod_info->big_values;
	ix += cod_info->big_values;
	
    assert(cod_info->count1table_select < 2);
	
    for(i = (cod_info->count1 - cod_info->big_values)>>2; i > 0; --i)
	{
		int huffbits = 0;
		int p = 0, v;
		
		v = ix[0];		
		p += v<<3;
		huffbits = sign_ptr[0]*v;
		assert(v <= 1u);
		
		v = ix[1];
		p += v<<2;
		huffbits <<= v;
		huffbits += sign_ptr[1]*v;
		assert(v <= 1u);
		
		v = ix[2];
		p += v<<1;
		huffbits <<= v;
		huffbits += sign_ptr[2]*v;
		assert(v <= 1u);
		
		v = ix[3];
		p += v;
		huffbits <<= v;
		huffbits += sign_ptr[3]*v;
		assert(v <= 1u);
		
		ix += 4;
		sign_ptr += 4;
		putbits(huffbits + h->table[p], h->hlen[p]);
		bits += h->hlen[p];
    }

    return bits;
}

//----------------------------------------------------------------------------------------------

/*Note the discussion of huffmancodebits() on pages 28
and 29 of the IS, as well as the definitions of the side
information on pages 26 and 27.*/
int Huffmancode(int tableindex,int start,int end,char *sign_ptr,short *ix)
{
    const struct huffcodetab *h = &ht[tableindex];
    int index, bits = 0;

    assert(tableindex < 32u);
    if (!tableindex)
	return bits;

    for(index = start; index < end; index += 2)
	{
		int cbits   = 0;
		int xbits   = 0;
		int linbits = h->xlen;
		int xlen    = h->xlen;
		int ext = 0;
		int x1 = ix[index];
		int x2 = ix[index+1];
		
		if(x1 != 0)
		{
			/*if(xr[index] < 0)
				ext++;*/
			ext = sign_ptr[index];
			cbits--;
		}
		
		if(tableindex > 15)
		{
			//use ESC-words
			if(x1 > 14)
			{
				int linbits_x1 = x1 - 15;
				assert(linbits_x1 <= h->linmax);
				ext   |= linbits_x1 << 1;
				xbits  = linbits;
				x1     = 15;
			}
			
			if(x2 > 14)
			{
				int linbits_x2 = x2 - 15;
				assert(linbits_x2 <= h->linmax);
				ext  <<= linbits;
				ext   |= linbits_x2;
				xbits += linbits;
				x2     = 15;
			}
			xlen = 16;
		}
		
		if(x2 != 0)
		{
			ext <<= 1;

			ext += sign_ptr[index+1];

			cbits--;
		}
		
		assert( (x1|x2) < 16u );
		
		x1 = x1 * xlen + x2;
		xbits -= cbits;
		cbits += h->hlen[x1];
				
		putbits(h->table[x1],cbits);
		putbits(ext,xbits);
		bits += cbits + xbits;
    }

    return bits;
}

//----------------------------------------------------------------------------------------------

int Huffmancodebits(char *sign_ptr,short *ix,gr_info *cod_info)
{
    int i,bigvalues,bits;
    int region1Start,region2Start;
	
    bigvalues = cod_info->big_values;
	
    i = cod_info->region0_count + 1;
    assert(i < 23);
    region1Start = scalefac_band_long[i];
    i += cod_info->region1_count + 1;
    assert(i < 23);
    region2Start = scalefac_band_long[i];
	
    if(region1Start > bigvalues)
		region1Start = bigvalues;
	
    if(region2Start > bigvalues)
		region2Start = bigvalues;
	
	bits  = Huffmancode(cod_info->table_select0, 0           , region1Start,sign_ptr,ix);

	bits += Huffmancode(cod_info->table_select1, region1Start, region2Start,sign_ptr,ix);

	bits += Huffmancode(cod_info->table_select2, region2Start, bigvalues,   sign_ptr,ix);

    return bits;
}

//----------------------------------------------------------------------------------------------

void encodeMainData(char *sign_ptr,short *mdct_enc,gr_info *side_info)
{
	gr_info *cod_info;
	short *enc_ptr;
    int gr,ch;
	int bits = -(main_data_begin<<3);
	int remainingbits,temp;

	scalefac_band_long = &sfBandIndex[config.mpeg.samplerate_index + (config.mpeg.type * 3)].l[0];
	
	for(gr=0;gr<config.mpeg.gr;gr++)
	{
		enc_ptr = mdct_enc + ((SAMP_PER_FRAME + DECDELAY)*gr);
		for(ch=0;ch<config.wave.channels;ch++)
		{
			cod_info = &side_info[(gr<<1)+ch];
			
			bits += Huffmancodebits(sign_ptr,enc_ptr,cod_info);
			
			bits += Huffman_coder_count1(sign_ptr,enc_ptr,cod_info);
			
			enc_ptr += SAMP_PER_FRAME_2;
			
			sign_ptr += SAMP_PER_FRAME_2;
		} /* for ch */
	} /* for gr */

	if(config.wave.channels==2)
		remainingbits = config.mpeg.bits_per_frame - bits - 288;
	else
		remainingbits = config.mpeg.bits_per_frame - bits - 168;
	
#if 0
	temp = remainingbits>>3;
	
	remainingbits -= (temp<<3);
	
	writeheader(-1,remainingbits);
	
	main_data_begin = temp;
	
	if(main_data_begin>100)
	{
		main_data_begin = MIN(main_data_begin,100);
		temp -= 100;
		while(temp--)
		{
			putbits(-1,8);
		}
	}
	
	resv_buf = &config.outbuf[outputsize];
	
	outputsize += main_data_begin;
#else
	temp = remainingbits>>5;

	remainingbits -= (temp<<5);

	while(temp--)
	{
		putbits(-1,32);
	}

	putbits(-1,remainingbits);
#endif
	
} /* main_data */


//----------------------------------------------------------------------------------------------


void encodeSideInfo(gr_info *cod_info)
{
	gr_info *gi;
    int gr,ch;
    
    writeheader(0xFFF,12);
	writeheader(config.mpeg.type,1);
	/* HEADER HARDCODED SHOULDN`T BE THIS WAY ! */
    writeheader(config.mpeg.layr,2);
    writeheader(!config.mpeg.crc,1);
    writeheader(config.mpeg.bitrate_index,4);
    writeheader(config.mpeg.samplerate_index,2);
    writeheader(config.mpeg.padding,1);
    writeheader(config.mpeg.ext,1);
    writeheader(config.mpeg.mode,2);
    writeheader(config.mpeg.mode_ext,2);
    writeheader(config.mpeg.copyright,1);
    writeheader(config.mpeg.original,1);
    writeheader(config.mpeg.emph,2);
    
	writeheader(0 /*si->main_data_end*/,9);
	//writeheader(main_data_begin,9);
	
	if(config.wave.channels == 2)
		writeheader(0/*si->private_bits*/,3);
	else
		writeheader(0/*si->private_bits*/,5);
	
	/*scfsi[scfsi_band] - in Layer III the scalefactor selection information works similarly to Layers I
	and II. The main difference is the use of the variable scfsi_band to apply scfsi to groups of scalefactors
	instead of single scalefactors. scfsi controls the use of scalefactors to the granules.
	'0' scalefactors are transmitted for each granule
	'1' scalefactors transmitted for granule 0 are also valid for_each granule 1*/

	for(ch=0;ch<config.wave.channels;ch++)
	{
		writeheader(0/*scfsi[scfsi_band]*/,4); // scfsi_band
	}
	
	for(gr=0;gr<2;gr++)
	{
		for(ch=0;ch<config.wave.channels;ch++)
		{
			gi =  &cod_info[(gr<<1)+ch];
			writeheader(gi->part2_3_length,12);
			writeheader(gi->big_values>>1,9);
			writeheader(gi->global_gain,8);
			
			writeheader(0/*gi->scalefac_compress*/,4);
			writeheader(0/*gi->window_switching_flag*/,1);
			
			if(gi->table_select0==14)
				gi->table_select0=16;
			writeheader(gi->table_select0,5);

			if(gi->table_select1==14)
				gi->table_select1=16;
			writeheader(gi->table_select1,5);

			if(gi->table_select2==14)
				gi->table_select2=16;
			writeheader(gi->table_select2,5);
			
			writeheader(gi->region0_count,4);
			writeheader(gi->region1_count,3);
			
			writeheader(0/*gi->preflag*/,1);
			writeheader(0/*gi->scalefac_scale*/,1);
			writeheader(gi->count1table_select,1);
		}
	}
}

//----------------------------------------------------------------------------------------------


void writeheader(unsigned int val,int len)
{
	int j;
	int k,tmp,putmask;
	
	if(len > 32)
		exit(-3);
	
	j = len + 7;
	j >>= 3;
	
	if(outputsize>528318)
		j = j;

	if(bit_count<len)
		j++;
	
	while(j--)
	{
		k = MIN(len,bit_count);
		len -= k;
		tmp = val>>len;
		bit_count -= k;
		putmask = 1<<k;
		putmask--;
		output += (tmp&putmask) << (bit_count);
		
		if(!bit_count)
		{
			config.outbuf[outputsize] = output;
			
			output = 0;
			bit_count = 8;
			
			outputsize++;
		}
	}
}

//----------------------------------------------------------------------------------------------

void putbits(unsigned int val,int len)
{
	int j;
	int k,tmp,putmask;
	
	if(len > 32)
		exit(-3);
	
	j = len + 7;
	j >>= 3;
	
	if(outputsize>528318)
		j = j;

	if(bit_count<len)
		j++;
	
	while(j--)
	{
		k = MIN(len,bit_count);
		len -= k;
		tmp = val>>len;
		bit_count -= k;
		putmask = 1<<k;
		putmask--;
		output += (tmp&putmask) << (bit_count);
		
		if(!bit_count)
		{
			config.outbuf[outputsize] = output;
			
			output = 0;
			bit_count = 8;
			
			outputsize++;
		}
	}
}

//----------------------------------------------------------------------------------------------
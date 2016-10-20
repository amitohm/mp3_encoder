#include "l3subband.h"


extern const short enwindow[];
extern int a[];

void window_subband(short *buf1,int *sb_sample_ptr)
{
	char sign = 1;
    int i;
	
    const short *wp = enwindow + 10;
	
	short *buf2 = &buf1[238-14-286];
	int w;
	int64_t s,t;
	int xr;
	int u,v;
	
    //for(i=-15;i<0;i++)
	for(i=0;i<15;i++)
	{
		w = wp[-10]; s = buf2[-224] * w; t  = buf1[ 224] * w;
		w = wp[-9]; s += buf2[-160] * w; t += buf1[ 160] * w;
		w = wp[-8]; s += buf2[- 96] * w; t += buf1[  96] * w;
		w = wp[-7]; s += buf2[- 32] * w; t += buf1[  32] * w;
		w = wp[-6]; s += buf2[  32] * w; t += buf1[- 32] * w;
		w = wp[-5]; s += buf2[  96] * w; t += buf1[- 96] * w;
		w = wp[-4]; s += buf2[ 160] * w; t += buf1[-160] * w;
		w = wp[-3]; s += buf2[ 224] * w; t += buf1[-224] * w;
		
		w = wp[-2]; s += buf1[-256] * w; t -= buf2[ 256] * w;
		w = wp[-1]; s += buf1[-192] * w; t -= buf2[ 192] * w;
		w = wp[ 0]; s += buf1[-128] * w; t -= buf2[ 128] * w;
		w = wp[ 1]; s += buf1[- 64] * w; t -= buf2[  64] * w;
		w = wp[ 2]; s += buf1[   0] * w; t -= buf2[   0] * w;
		w = wp[ 3]; s += buf1[  64] * w; t -= buf2[- 64] * w;
		w = wp[ 4]; s += buf1[ 128] * w; t -= buf2[-128] * w;
		w = wp[ 5]; s += buf1[ 192] * w; t -= buf2[-192] * w;
		
		/*this multiplyer could be removed, but it needs more 256 FLOAT data.
		thinking about the data cache performance, I think we should not
		use such a huge table. tt 2000/Oct/25*/

		sign = 1;
		if(s<0)
			sign = -1;
		s *= sign;
		s >>= 10;
		s *= wp[6];
		s >>= 10;
		s *= sign;

		sign = 1;
		if(t<0)
			sign = -1;
		t *= sign;
		t >>= 10;
		t *= sign;
		
		
		a[i*2] = (int)(t + s);
		
		t -= s;
		sign = 1;
		if(t<0)
			sign = -1;
		t *= sign;
		t *= wp[7];
		t >>= 10;
		t *= sign;
		a[i*2+1] = (int)t;
		wp += 18;
		buf1--;
		buf2++;
    }
	
	t  =  buf1[- 16] * wp[-10];                s  = buf1[ -32] * wp[-2];
	t += (buf1[- 48] - buf1[ 16]) * wp[-9];    s += buf1[ -96] * wp[-1];
	t += (buf1[- 80] + buf1[ 48]) * wp[-8];    s += buf1[-160] * wp[ 0];
	t += (buf1[-112] - buf1[ 80]) * wp[-7];    s += buf1[-224] * wp[ 1];
	t += (buf1[-144] + buf1[112]) * wp[-6];    s -= buf1[  32] * wp[ 2];
	t += (buf1[-176] - buf1[144]) * wp[-5];    s -= buf1[  96] * wp[ 3];
	t += (buf1[-208] + buf1[176]) * wp[-4];    s -= buf1[ 160] * wp[ 4];
	t += (buf1[-240] - buf1[208]) * wp[-3];    s -= buf1[ 224] * wp[ 5];
	
	sign = 1;
	if(s<0)
		sign = -1;
	s *= sign;
	s >>= 10;
	s *= sign;
	sign = 1;
	if(t<0)
		sign = -1;
	t *= sign;
	t >>= 10;
	t *= sign;
	
	u = (int)(s - t);
	v = (int)(s + t);
	
	t = a[14];
	s = a[15] - t;
	
	a[31] = (int)(v + t);   /* A0 */
	a[30] = (int)(u + s);   /* A1 */
	a[15] = (int)(u - s);   /* A2 */
	a[14] = (int)(v - t);   /* A3 */
	
	xr = a[28] - a[ 0];xr >>= 10; a[ 0] += a[28]; a[28] = xr * wp[-2*18+7];
	xr = a[29] - a[ 1];xr >>= 10; a[ 1] += a[29]; a[29] = xr * wp[-2*18+7];
	
	xr = a[26] - a[ 2];xr >>= 10; a[ 2] += a[26]; a[26] = xr * wp[-4*18+7];
	xr = a[27] - a[ 3];xr >>= 10; a[ 3] += a[27]; a[27] = xr * wp[-4*18+7];
	
	xr = a[24] - a[ 4];xr >>= 10; a[ 4] += a[24]; a[24] = xr * wp[-6*18+7];
	xr = a[25] - a[ 5];xr >>= 10; a[ 5] += a[25]; a[25] = xr * wp[-6*18+7];
	
	xr = a[22] - a[ 6];xr >>= 10; a[ 6] += a[22]; a[22] = xr * wp[-8*18+7];
	xr = a[23] - a[ 7];xr >>= 10; a[ 7] += a[23]; a[23] = xr * wp[-8*18+7];
	a[23] -= a[ 7];
	a[ 7] -= a[ 6];
	a[22] -= a[ 7];
	a[23] -= a[22];
	
	xr = a[ 6]; a[ 6] = a[31] - xr; a[31] = a[31] + xr;
	xr = a[ 7]; a[ 7] = a[30] - xr; a[30] = a[30] + xr;
	xr = a[22]; a[22] = a[15] - xr; a[15] = a[15] + xr;
	xr = a[23]; a[23] = a[14] - xr; a[14] = a[14] + xr;
	
	xr = a[20] - a[ 8];xr >>= 10; a[ 8] += a[20]; a[20] = xr * wp[-10*18+7];
	xr = a[21] - a[ 9];xr >>= 10; a[ 9] += a[21]; a[21] = xr * wp[-10*18+7];
	
	xr = a[18] - a[10];xr >>= 10; a[10] += a[18]; a[18] = xr * wp[-12*18+7];
	xr = a[19] - a[11];xr >>= 10; a[11] += a[19]; a[19] = xr * wp[-12*18+7];
	
	xr = a[16] - a[12];xr >>= 10; a[12] += a[16]; a[16] = xr * wp[-14*18+7];
	xr = a[17] - a[13];xr >>= 10; a[13] += a[17]; a[17] = xr * wp[-14*18+7];
	
	xr = -a[20] + a[24];xr >>= 10; a[20] += a[24]; a[24] = xr * wp[-12*18+7];
	xr = -a[21] + a[25];xr >>= 10; a[21] += a[25]; a[25] = xr * wp[-12*18+7];
	
	xr = a[ 4] - a[ 8];xr >>= 10; a[ 4] += a[ 8]; a[ 8] = xr * wp[-12*18+7];
	xr = a[ 5] - a[ 9];xr >>= 10; a[ 5] += a[ 9]; a[ 9] = xr * wp[-12*18+7];
	
	xr = a[ 0] - a[12];xr >>= 10; a[ 0] += a[12]; a[12] = xr * wp[-4*18+7];
	xr = a[ 1] - a[13];xr >>= 10; a[ 1] += a[13]; a[13] = xr * wp[-4*18+7];
	xr = a[16] - a[28];xr >>= 10; a[16] += a[28]; a[28] = xr * wp[-4*18+7];
	xr = -a[17] + a[29];xr >>= 10; a[17] += a[29]; a[29] = xr * wp[-4*18+7];
	
	xr = a[ 2] - a[10];xr >>= 10; a[ 2] += a[10]; a[10] = xr * wp[-8*18+7];
	xr = a[ 3] - a[11];xr >>= 10; a[ 3] += a[11]; a[11] = xr * wp[-8*18+7];
	xr = -a[18] + a[26];xr >>= 10; a[18] += a[26]; a[26] = xr * wp[-8*18+7];
	a[26] -= a[18];
	xr = -a[19] + a[27];xr >>= 10; a[19] += a[27]; a[27] = xr * wp[-8*18+7];
	a[27] -= a[19];
	
	xr = a[ 2]; a[19] -= a[ 3]; a[ 3] -= xr; a[ 2] = a[31] - xr; a[31] += xr;
	xr = a[ 3]; a[11] -= a[19]; a[18] -= xr; a[ 3] = a[30] - xr; a[30] += xr;
	xr = a[18]; a[27] -= a[11]; a[19] -= xr; a[18] = a[15] - xr; a[15] += xr;
	
	xr = a[19]; a[10] -= xr; a[19] = a[14] - xr; a[14] += xr;
	xr = a[10]; a[11] -= xr; a[10] = a[23] - xr; a[23] += xr;
	xr = a[11]; a[26] -= xr; a[11] = a[22] - xr; a[22] += xr;
	xr = a[26]; a[27] -= xr; a[26] = a[ 7] - xr; a[ 7] += xr;
	
	xr = a[27]; a[27] = a[ 6] - xr; a[ 6] += xr;
	
	xr = a[ 0] - a[ 4];xr >>= 10; a[ 0] += a[ 4]; a[ 4] = xr * wp[-8*18+7];
	xr = a[ 1] - a[ 5];xr >>= 10; a[ 1] += a[ 5]; a[ 5] = xr * wp[-8*18+7];
	xr = a[16] - a[20];xr >>= 10; a[16] += a[20]; a[20] = xr * wp[-8*18+7];
	xr = a[17] - a[21];xr >>= 10; a[17] += a[21]; a[21] = xr * wp[-8*18+7];
	
	xr = -a[ 8] + a[12];xr >>= 10; a[ 8] += a[12]; a[12] = xr * wp[-8*18+7];
	a[12] -= a[ 8];
	xr = -a[ 9] + a[13];xr >>= 10; a[ 9] += a[13]; a[13] = xr * wp[-8*18+7];
	a[13] -= a[ 9];
	xr = -a[25] + a[29];xr >>= 10; a[25] += a[29]; a[29] = xr * wp[-8*18+7];
	a[29] -= a[25];
	xr = -a[24] - a[28];xr >>= 10; a[24] -= a[28]; a[28] = xr * wp[-8*18+7];
	a[28] -= a[24];
	
	xr = a[24] - a[16]; a[24] = xr;
	xr = a[20] - xr;    a[20] = xr;
	xr = a[28] - xr;    a[28] = xr;
	
	xr = a[25] - a[17]; a[25] = xr;
	xr = a[21] - xr;    a[21] = xr;
	xr = a[29] - xr;    a[29] = xr;
	
	xr = a[17] - a[ 1]; a[17] = xr;
	xr = a[ 9] - xr;    a[ 9] = xr;
	xr = a[25] - xr;    a[25] = xr;
	xr = a[ 5] - xr;    a[ 5] = xr;
	xr = a[21] - xr;    a[21] = xr;
	xr = a[13] - xr;    a[13] = xr;
	xr = a[29] - xr;    a[29] = xr;
	
	xr = a[ 1] - a[ 0]; a[ 1] = xr;
	xr = a[16] - xr;    a[16] = xr;
	xr = a[17] - xr;    a[17] = xr;
	xr = a[ 8] - xr;    a[ 8] = xr;
	xr = a[ 9] - xr;    a[ 9] = xr;
	xr = a[24] - xr;    a[24] = xr;
	xr = a[25] - xr;    a[25] = xr;
	xr = a[ 4] - xr;    a[ 4] = xr;
	xr = a[ 5] - xr;    a[ 5] = xr;
	xr = a[20] - xr;    a[20] = xr;
	xr = a[21] - xr;    a[21] = xr;
	xr = a[12] - xr;    a[12] = xr;
	xr = a[13] - xr;    a[13] = xr;
	xr = a[28] - xr;    a[28] = xr;
	xr = a[29] - xr;    a[29] = xr;
	
	xr = a[ 0]; a[ 0] += a[31]; a[31] -= xr;
	xr = a[ 1]; a[ 1] += a[30]; a[30] -= xr;
	xr = a[16]; a[16] += a[15]; a[15] -= xr;
	xr = a[17]; a[17] += a[14]; a[14] -= xr;
	xr = a[ 8]; a[ 8] += a[23]; a[23] -= xr;
	xr = a[ 9]; a[ 9] += a[22]; a[22] -= xr;
	xr = a[24]; a[24] += a[ 7]; a[ 7] -= xr;
	xr = a[25]; a[25] += a[ 6]; a[ 6] -= xr;
	xr = a[ 4]; a[ 4] += a[27]; a[27] -= xr;
	xr = a[ 5]; a[ 5] += a[26]; a[26] -= xr;
	xr = a[20]; a[20] += a[11]; a[11] -= xr;
	xr = a[21]; a[21] += a[10]; a[10] -= xr;
	xr = a[12]; a[12] += a[19]; a[19] -= xr;
	xr = a[13]; a[13] += a[18]; a[18] -= xr;
	xr = a[28]; a[28] += a[ 3]; a[ 3] -= xr;
	xr = a[29]; a[29] += a[ 2]; a[ 2] -= xr;
	
	for(i=0;i<32;i++)
	{
		sb_sample_ptr[i] = a[i]>>10;
	}
}

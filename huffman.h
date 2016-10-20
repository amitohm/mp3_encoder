
#define NULL    ((void *)0)

#define HTN	34

struct huffcodetab {
    const int    xlen; 	        /* max. x-index+			*/ 
    const int    linmax;	/* max number to be stored in linbits	*/
    const short*   table;	        /* pointer to array[xlen][ylen]		*/
    const char*  hlen;	        /* pointer to array[xlen][ylen]		*/
};

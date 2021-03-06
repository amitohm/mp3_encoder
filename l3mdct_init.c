#include "l3mdct.h"

const short win[NL] = 
{
	244,
	657,
	963,
	1149,
	1212,
	1150,
	963,
	657,
	244,

	5587,
	4996,
	4342,
	3645,
	2927,
	2208,
	1511,
	857,
	266,

	-5586,
	-4995,
	-4341,
	-3644,
	-2926,
	-2207,
	-1510,
	-856,
	-265,

	-243,
	-656,
	-962,
	-1148,
	-1211,
	-1148,
	-962,
	-656,
	-243
};

const short tantab_l[9] = 
{
	1117,	// tantab_l
	1334,
	1608,
	1967,
	2472,
	3248,
	4619,
	7778,
	23454
};

const short cx[8] = 
{
	1009,	// cx
	658,
	350,
	963,
	-178,
	-784,
	887,
	512
};

const short ca[8] = 
{
	-527,	//ca
	-483,
	-321,
	-186,
	-97,
	-42,
	-14,
	-4
};

const short cs[8] = 
{
	879,	//cs
	903,
	973,
	1007,
	1020,
	1023,
	1024,
	1024
};


int order[] = {	0,		1,
				16,		17,
				8,		9,
				24,		25,
				4,		5,
				20,		21,
				12,		13,
				28,		29,
				2,		3,
				18,		19,
				10,		11,
				26,		27,
				6,		7,
				22,		23,
				14,		15,
				30,		31};
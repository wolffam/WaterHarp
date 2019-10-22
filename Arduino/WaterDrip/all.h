#include "shd.h"
#include "mary.h"
#include "smiley.h"
#include "sine.h"
#include "bars.h"
#include "rain.h"

unsigned short *data[] = {rain,mary, shd,smiley,sine,bars};
int len[]={
    sizeof(rain)/sizeof(rain[0]),
    sizeof(mary)/sizeof(mary[0]),
	   sizeof(shd)/sizeof(shd[0]),
	   sizeof(smiley)/sizeof(smiley[0]),
	   sizeof(sine)/sizeof(sine[0]),
	   sizeof(bars)/sizeof(bars[0])};
char *names[]={"rain","mary","shd","smiley","sine","bars"};
int step[]={5,10,20,40,20,40};
int ndata = sizeof(data)/sizeof(data[0]);
int del[]={0,1000,1000,1000,0,0};

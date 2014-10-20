#include "Arduino.h"

#ifndef define_h
#define define_h

#define LED_COUNT 110
#define REFRESH_RATE 20    // in ms
int HourMask[66] = {
  16,16,16,16,0,8191,8191,8191,0,8191,8191,8,8,8,8,8,8,4,4,4,4,4,64,64,64,96,512,0,2048,256,256,256,256,1,1,1,4096,6144,4096,4096,4640,4096,0,0,0,0,512,34,2,2,2050,1024,1024,1024,1024,128,128,128,128,2048,2047,2047,2047,2047,2047,2046
};

int MinuteMask[44] = {
  514,514,514,256,256,0,1984,1984,1984,1984,1984,4,4,0,260,260,260,260,260,0,0,0,1105,1105,1105,1105,80,216,216,216,216,216,0,32,32,0,32,32,32,32,32,0,0,0
};

int Mapping[110] = {
  99,100,101,102,103,104,105,106,107,108,109,88,89,90,91,92,93,94,95,96,97,98,77,78,79,80,81,82,83,84,85,86,87,66,67,68,69,70,71,72,73,74,75,76,55,56,57,58,59,60,61,62,63,64,65,44,45,46,47,48,49,50,51,52,53,54,33,34,35,36,37,38,39,40,41,42,43,22,23,24,25,26,27,28,29,30,31,32,11,12,13,14,15,16,17,18,19,20,21,0,1,2,3,4,5,6,7,8,9,10
};

#endif



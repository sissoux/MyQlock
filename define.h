#include "Arduino.h"

#ifndef define_h
#define define_h

#define LED_COUNT 110
#define REFRESH_RATE 20    // in ms
#define COLOR_CHANGE_RATE 25
#define INPUT_REFRESH_RATE 20
#define BLINK_RATE 500

#define R_SENSOR_THRESHOLD 600
#define L_SENSOR_THRESHOLD 600
#define DEBOUNCE_DELAY 200
#define R_FLAG_MASK 0b00000001
#define L_FLAG_MASK 0b00000010

#define NUMBER_OF_STATIC_COLORS 1


int HourMask[66] = {
  16,16,16,16,0,8191,8191,8191,0,8191,8191,8,8,8,8,8,8,4,4,4,4,4,64,64,64,96,512,0,2048,256,256,256,256,1,1,1,4096,6144,4096,4096,4640,4096,0,0,0,0,512,34,2,2,2050,1024,1024,1024,1024,128,128,128,128,2048,2047,2047,2047,2047,2047,2046
};

int MinuteMask[44] = {
  514,514,514,256,256,0,1984,1984,1984,1984,1984,4,4,0,260,260,260,260,260,0,0,0,1105,1105,1105,1105,80,216,216,216,216,216,0,32,32,0,32,32,32,32,32,0,0,0
};

int Mapping[110] = {
  99,100,101,102,103,104,105,106,107,108,109,88,89,90,91,92,93,94,95,96,97,98,77,78,79,80,81,82,83,84,85,86,87,66,67,68,69,70,71,72,73,74,75,76,55,56,57,58,59,60,61,62,63,64,65,44,45,46,47,48,49,50,51,52,53,54,33,34,35,36,37,38,39,40,41,42,43,22,23,24,25,26,27,28,29,30,31,32,11,12,13,14,15,16,17,18,19,20,21,0,1,2,3,4,5,6,7,8,9,10
};

#define X 0
#define Y 1
#define R 2
#define G 3
#define B 4

#define H_FRAME 1
uint8_t FrameH[][5] = {{3,1,255,255,192},{7,1,255,255,192},{3,2,255,255,192},{7,2,255,255,192},{3,3,255,255,192},{7,3,255,255,192},{3,4,255,255,192},{4,4,255,255,192},{5,4,255,255,192},{6,4,255,255,192},{7,4,255,255,192},{3,5,255,255,192},{7,5,255,255,192},{3,6,255,255,192},{7,6,255,255,192},{3,7,255,255,192},{7,7,255,255,192}};

#define M_FRAME 2
uint8_t FrameM[][5] = {{2,0,255,255,255},{8,0,255,255,255},{2,1,255,255,255},{3,1,255,255,255},{7,1,255,255,255},{8,1,255,255,255},{2,2,255,255,255},{4,2,255,255,255},{6,2,255,255,255},{8,2,255,255,255},{2,3,255,255,255},{5,3,255,255,255},{8,3,255,255,255},{2,4,255,255,255},{8,4,255,255,255},{2,5,255,255,255},{8,5,255,255,255},{2,6,255,255,255},{8,6,255,255,255}};

#define HI_FRAME 3
uint8_t FrameHI[][5] = {{2,2,255,255,255},{5,2,255,255,255},{8,2,255,255,255},{2,3,255,255,255},{5,3,255,255,255},{2,4,255,255,255},{3,4,255,255,255},{4,4,255,255,255},{5,4,255,255,255},{8,4,255,255,255},{2,5,255,255,255},{5,5,255,255,255},{8,5,255,255,255},{2,6,255,255,255},{5,6,255,255,255},{8,6,255,255,255}};

#define BYE_FRAME 4
uint8_t FrameBYE[][5] = {{0,2,255,255,255},{1,2,255,255,255},{4,2,255,255,255},{6,2,255,255,255},{8,2,255,255,255},{9,2,255,255,255},{10,2,255,255,255},{0,3,255,255,255},{2,3,255,255,255},{4,3,255,255,255},{6,3,255,255,255},{8,3,255,255,255},{0,4,255,255,255},{1,4,255,255,255},{5,4,255,255,255},{8,4,255,255,255},{9,4,255,255,255},{0,5,255,255,255},{2,5,255,255,255},{5,5,255,255,255},{8,5,255,255,255},{0,6,255,255,255},{1,6,255,255,255},{5,6,255,255,255},{8,6,255,255,255},{9,6,255,255,255},{10,6,255,255,255}};

#define RTC_FRAME 5
//uint8_t FrameRTC[][5]











    typedef struct {
    double r;       // percent
    double g;       // percent
    double b;       // percent
} rgb;

    typedef struct {
    double h;       // angle in degrees
    double s;       // percent
    double v;       // percent
} hsv;

    static hsv      rgb2hsv(rgb in);
    static rgb      hsv2rgb(hsv in);

hsv rgb2hsv(rgb in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
            // s = 0, v is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}


rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;     
}



#endif


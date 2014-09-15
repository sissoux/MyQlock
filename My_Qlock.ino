#include "define.h"
#include <PololuLedStrip.h>

PololuLedStrip<13> LEDStrip;

rgb_color buffer[LED_COUNT];

int j = 0;

void setup()
{
  for (int i = 0 ; i < LED_COUNT ; i++)
  {
    buffer[i].red = 0;
    buffer[i].green = 0;
    buffer[i].blue = 0;
  }
  LEDStrip.write(buffer, LED_COUNT);
}

void loop()
{
  int heure = 10;
  for (int i = 0 ; i < 66 ; i++)
  {
    if ((LEDString[i]>>heure-1)&1)
    {
      buffer[i].red = 255;
      buffer[i].green = 255;
      buffer[i].blue = 255;
    }
    else if (i  == 10 || i == 9|| i == 7|| i == 6|| i == 5)
    {
      buffer[i].red = 20*j +50 ;
      buffer[i].green = 20*j + 50;
      buffer[i].blue = 20*j + 50 ;
    }
    else
    
    {
      buffer[i].red = 0;
      buffer[i].green = 0;
      buffer[i].blue = 0;
    }
  }
  
  LEDStrip.write(buffer, LED_COUNT);
  delay(1000);
  j = (j + 1);
  if(j == 5) j=0;
}

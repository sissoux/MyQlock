#include <Adafruit_NeoPixel.h>
#include <Time.h>  
#include "define.h"
#include <CapacitiveSensor.h>



Adafruit_NeoPixel strip = Adafruit_NeoPixel(110, 6, NEO_GRB + NEO_KHZ800);    // Declare the LED strip using Adafruit library


CapacitiveSensor   RcapSens = CapacitiveSensor(22,19);        // Right capacitive sensor
CapacitiveSensor   LcapSens = CapacitiveSensor(22,18);        // Left capacitive sensor


boolean Error = false;            // Error state
boolean RefreshFlag = false;      // Used to tell if strip has been refreshed

uint32_t OutBuffer_0[LED_COUNT];    // First buffer of 2 for circular buffering. Contains Pixel color
uint32_t OutBuffer_1[LED_COUNT];    // Second buffer of 2 for circular buffering. Contains Pixel color
int ActiveBuffer = 0;               // Contains the active (to be displayed) buffer ID

elapsedMillis RefreshTimer;          // auto incremented value, called to sequence and synchronize tasks ==> Output refresh rate
elapsedMillis ColorTimer;          

uint16_t h = 0;                      // Temp var used to store current Hour
uint16_t m = 0;                      // Temp var used to store current Minute
uint32_t ColorMask;

rgb MyRGBColor;
hsv MyHSVColor;


enum colormode{
FADING,
STATIC
};

colormode ColorMode = FADING;


void setup()  {
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);    // Set the clock reference to use the 32.768kHz crystal of the Teensy

  Serial.begin(115200);               // For debug purpose, or for future APP dev
  while (!Serial);                    // Wait for Arduino Serial Monitor to open For debug purpose

  delay(100);                         // Useless Delay, but on sait jamais ^^

  if (timeStatus()!= timeSet)         // Check for RTC sync error
  {
    Serial.println("Unable to sync with the RTC");
    Error = true;
  }
  ColorMask = strip.Color(100,25,0);
  strip.begin();                    // Start the strip
  strip.show();                     // Initialize all pixels to 'off'
  MyHSVColor.h = 0.0;
  MyHSVColor.s = 1.0;
  MyHSVColor.v = 1.0;
}

void loop() {
  
  //// Display refresh check section ////
  if (RefreshTimer >= REFRESH_RATE) 
  {
    RefreshTimer = RefreshTimer - REFRESH_RATE;
    refreshOutput();
    RefreshFlag = 1;
    //m++;
    //if (m == 60) m = 0;
    writeTime();
    
  }
  
  //// Display refresh check section ////
  if (ColorTimer >= COLOR_CHANGE_RATE) 
  {
    ColorTimer = ColorTimer - COLOR_CHANGE_RATE;
    //ActiveBuffer = !ActiveBuffer;
    //h++;
    //if (h == 14) h = 1;
    if (false)//ColorMode == FADING)
    {
      ColorMask+=10;
      MyHSVColor.h += 1;//0.1;
      if (MyHSVColor.h==360) MyHSVColor.h = 0;
      Serial.println(MyHSVColor.h);
      MyRGBColor = hsv2rgb(MyHSVColor);
      ColorMask = strip.Color((int)(MyRGBColor.r*255), (int)(MyRGBColor.g*255), (int)(MyRGBColor.b*255));
    }
    //Serial.println((double)(constrain(map(analogRead(A9),300,900,100,10),10,100))/100.0);
    MyHSVColor.v = (double)(constrain(map(analogRead(A9),300,900,100,10),10,100))/100.0;
    MyRGBColor = hsv2rgb(MyHSVColor);
    ColorMask = strip.Color((int)(MyRGBColor.r*255), (int)(MyRGBColor.g*255), (int)(MyRGBColor.b*255));
  }


  if(RcapSensor > R_SENSOR_THRESHOLD)
  {
    while (RcapSensor > R_SENSOR_THRESHOLD);
    Now = millis();
    while (millis() - Now < DEBOUNCE_DELAY);
    Serial.println("Got Touch on RIGHT Sensor");
  }
  
  if(LcapSensor > L_SENSOR_THRESHOLD)
  {
    while (LcapSensor > L_SENSOR_THRESHOLD);
    Now = millis();
    while (millis() - Now < DEBOUNCE_DELAY);
    Serial.println("Got Touch on LEFT Sensor");
  }
  Serial.println("No Touch :(");
  //// Display refresh check section ////
}


void refreshOutput()
{
  uint32_t TempColor;             // For debug purpose
  
  if (!ActiveBuffer)                              // If active buffer = 0 then use OutBuffer_0
  {
    for(uint16_t i=0; i<strip.numPixels(); i++) 
    {
      /*TempColor = strip.Color(10,0,0);
      strip.setPixelColor(i, TempColor);*/
      strip.setPixelColor(i, OutBuffer_0[Mapping[i]]);
    }
  }
  else
  {
    for(uint16_t i=0; i<strip.numPixels(); i++) 
    {
      /*TempColor = strip.Color(0,10,0);
      strip.setPixelColor(i, TempColor);*/
      strip.setPixelColor(i, OutBuffer_1[Mapping[i]]); 
    }
  }
  
  strip.show();                // Display selected buffer
}

time_t getTeensy3Time()      // Function used by RTC
{
  return Teensy3Clock.get();
}


void writeTime()
{
  h = 9;//hour()%12;
  //Serial.print(h);
  m = 30; //minute();
  //Serial.print("  ");
  //Serial.println(m);
  int FithOfMin = m / 5;
  clear_Buffer(ActiveBuffer);
  for (int pixel = 0 ; pixel < 66 ; pixel++)
  {
    if ((HourMask[pixel]>>h-1)&1)
    {
      fill_Buffer(ActiveBuffer, pixel, ColorMask);
    }    
  }
  for (int pixel = 66 ; pixel < 110 ; pixel++)
  {
    if ((MinuteMask[pixel-66]>>FithOfMin-1)&1)
    {
      fill_Buffer(ActiveBuffer, pixel, ColorMask);
    }    
  }
  
  
}

void fill_Buffer(int BufferID, int PixelID, uint32_t Color)
{
  if (!BufferID)
  {
    OutBuffer_0[PixelID] = Color;
  }
  else
  {
    OutBuffer_1[PixelID] = Color;
  }
}

uint32_t get_Buffer(int BufferID, int PixelID)
{
  if (!BufferID)
  {
    return OutBuffer_0[PixelID];
  }
  else
  {
    return OutBuffer_1[PixelID];
  }
}

void clear_Buffer(int BufferID)
{
  if (!BufferID)
  {
    for (int Pixel = 0; Pixel<110; Pixel++)
    {
      OutBuffer_0[Pixel] = 0;
    }
  }
  else
  {for (int Pixel = 0; Pixel<110; Pixel++)
    {
      OutBuffer_1[Pixel] = 0;
    }
  }
}



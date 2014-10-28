#include <Adafruit_NeoPixel.h>
#include <Time.h>  
#include "define.h"
#include <CapacitiveSensor.h>

//TODO Faire une distinction clic long, clic rapide.
//TODO Effet Matrix
//TODO Indiquer un temps par minute
//TODO Réglage luminosité maximale
//TODO Messages / Génération de message
//TODO Optimiser

Adafruit_NeoPixel strip = Adafruit_NeoPixel(110, 6, NEO_GRB + NEO_KHZ800);    // Declare the LED strip using Adafruit library

CapacitiveSensor   RcapSensor = CapacitiveSensor(22,18);        // Right capacitive sensor
CapacitiveSensor   LcapSensor = CapacitiveSensor(22,19);        // Left capacitive sensor
int TouchCounter = 0;
int screenRefreshRate = 20;

byte FlagRegister = 0;
byte Blink = 0;

boolean RTCError = false;            // Error state
//boolean RefreshFlag = false;      // Used to tell if strip has been refreshed

uint32_t OutBuffer_0[LED_COUNT];    // First buffer of 2 for circular buffering. Contains Pixel color
uint32_t OutBuffer_1[LED_COUNT];    // Second buffer of 2 for circular buffering. Contains Pixel color
int ActiveBuffer = 0;               // Contains the active (to be displayed) buffer ID

elapsedMillis RefreshTimer;          // auto incremented value, called to sequence and synchronize tasks ==> Output refresh rate
elapsedMillis ColorTimer;        
elapsedMillis InputRefreshTimer;     
elapsedMillis BlinkerTimer;          

uint16_t h = 1;                      // Temp var used to store current Hour
uint16_t m = 0;                      // Temp var used to store current Minute
uint32_t ColorMask;

rgb MyRGBColor;
hsv MyHSVColor;
int SatSign = -1;

boolean hasToBeReDrawnNow = false;

state CurrentState = NORMAL;
colormode ColorMode = FADING;

enum colormode{
  FADING,
  STATIC
};

enum state{
  NORMAL,
  HSET,
  MSET,
  LUMSET,
  LUMSENS,
  STBY
};

void setup()  {
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);    // Set the clock reference to use the 32.768kHz crystal of the Teensy

  Serial.begin(115200);               // For debug purpose, or for future APP dev
  //while (!Serial);                    // Wait for Arduino Serial Monitor to open For debug purpose

  if (timeStatus()!= timeSet)         // Check for RTC sync error
  {
    Serial.println("Unable to sync with the RTC");
    RTCError = true;
  }
  ColorMask = strip.Color(0,0,0);
  strip.begin();                    // Start the strip
  strip.show();                     // Initialize all pixels to 'off'

  colorWipe(strip.Color(0, 0, 255), 15); // Blue
  colorWipe(strip.Color(0, 0, 0), 15); // Blue
  rainbow(1);
  rainbow(1);
  rainbow(1);

  writeFrame(HI_FRAME);
  
  MyHSVColor.h = 150.0;
  MyHSVColor.s = 1.0;
  MyHSVColor.v = 1.0;
  
	if (ColorMode == STATIC)
	{
		//We use a slow refresh rate
		screenRefreshRate = 500;
	}
	else
	{
		//We can use a fast refresh rate
		screenRefreshRate = REFRESH_RATE;
	}
}

void loop()
{
	if (CurrentState != STBY)
	{
		//Here, you'll find everything related to graphics because it matters only when != STBY
		if (RefreshTimer >= screenRefreshRate || hasToBeReDrawnNow) 
		{
			RefreshTimer = 0;
			writeTime();
			refreshOutput();
			hasToBeReDrawnNow = false;
			//RefreshFlag = true;
		}
		if (ColorTimer >= COLOR_CHANGE_RATE) 
		{
			ColorTimer = 0;
			//Performance : Do not fill stack
			if (ColorMode == FADING)
			{
				colorMaskUpdate();
				hasToBeReDrawnNow = true;
			}
		}
		if (BlinkerTimer >= BLINK_RATE) 
		{
			BlinkerTimer = 0;
			Blink = !Blink;
			hasToBeReDrawnNow = true;
		}
	}
	
	if (InputRefreshTimer >= INPUT_REFRESH_RATE) 
	{
		InputRefreshTimer = 0;
		sensorCheck();
		navig();
	}
}

void delay2(int ms)
{
	unsigned long Now = millis();
	while (millis() - Now < ms);
}

void navig()
{
	switch (CurrentState)
	{
		case NORMAL:
		if(FlagRegister & (R_FLAG_MASK | L_FLAG_MASK))
		{
			CurrentState = HSET;
			writeFrame(H_FRAME);
			delay2(1500);
		}
		else if (FlagRegister & L_FLAG_MASK)
		{
			if (TouchCounter >= 5)
			{
				CurrentState = STBY;
				writeFrame(BYE_FRAME);
				delay2(1500);
			}
			else 
			{
				TouchCounter++;
			}
		}
		else
		{
		  TouchCounter = 0;
		}
		break;
		
		case HSET:
		if (FlagRegister & L_FLAG_MASK)
		{
			CurrentState = MSET;
			writeFrame(M_FRAME);
			delay2(1500);
		}
		if (FlagRegister & R_FLAG_MASK)
		{
			Teensy3Clock.set((time_t)(now()+3600));
			setTime(hour()+1,minute(),second(),day(),month(),year());
		}
		break;

		case MSET:
		if (FlagRegister & L_FLAG_MASK)
		{
			CurrentState = NORMAL;
		}
		if (FlagRegister & R_FLAG_MASK)
		{
			Teensy3Clock.set((time_t)(now()+300));
			setTime(hour(),minute()+5,second(),day(),month(),year());
		}    
		break;

		case LUMSET:
		if (FlagRegister & L_FLAG_MASK)
		{
			CurrentState = NORMAL;
		}
		break;

		case LUMSENS:
		if (FlagRegister & L_FLAG_MASK)
		{
			CurrentState = NORMAL;
		}
		break;  

		case STBY:
		if (FlagRegister & L_FLAG_MASK)
		{
			CurrentState = NORMAL;
			writeFrame(HI_FRAME);
			delay2(1500);
		}
		break;    
	}
	FlagRegister &= ~(R_FLAG_MASK | L_FLAG_MASK);
}

void colorMaskUpdate()
{
	if (ColorMode == FADING)
	{
		MyHSVColor.h += 0.02;
		if (MyHSVColor.h>360) MyHSVColor.h = 0;

		MyHSVColor.v = (double)(constrain(map(analogRead(A9),300,900,100,5),5,100))/100.0;

		MyHSVColor.s += 0.000003*SatSign;
		if (MyHSVColor.s > 1) SatSign = -1;
		if (MyHSVColor.s < 0) SatSign = 1;

		MyRGBColor = hsv2rgb(MyHSVColor);
		ColorMask = strip.Color((int)(MyRGBColor.r*255), (int)(MyRGBColor.g*255), (int)(MyRGBColor.b*255));
	}
}

//// Capacitive sensor thread ////
void sensorCheck()
{
	long LcapSensorVal =  LcapSensor.capacitiveSensor(30);      //Launch capacitive sensor acquisition, return corresponding delay
	delay2(50);
	long RcapSensorVal =  RcapSensor.capacitiveSensor(30);

	if(RcapSensorVal > R_SENSOR_THRESHOLD)      // If returned value is greater than Threshold, debounce and raise corresponding FLAG
	{
		delay2(200);
		FlagRegister |= R_FLAG_MASK;
	}

	if(LcapSensorVal > L_SENSOR_THRESHOLD)
	{
		delay2(200);
		FlagRegister |= L_FLAG_MASK;
	}
}

void refreshOutput()
{
  //uint32_t TempColor;             // For debug purpose
  
	uint32_t OutBuffer = ActiveBuffer ? OutBuffer_1 : OutBuffer_0;

	for(uint16_t i=0; i<strip.numPixels(); i++) 
	{
		strip.setPixelColor(i, OutBuffer[Mapping[i]]); 
	}

	strip.show();                // Display selected buffer
}

time_t getTeensy3Time()      // Function used by RTC
{
	return Teensy3Clock.get();
}

void writeTime()
{
	m = minute();
	h = hour();
	if (m >=35)
	{
		h++;
	}
	h = h%12;
	if (h == 0)
	{
		h = isPM() ? 13 : 12;
	}

	int FithOfMin = m / 5;
	clear_Buffer(ActiveBuffer);

	for (int pixel = 0 ; pixel < 66 ; pixel++)            // Fill in HOUR pixels
	{
		if ((HourMask[pixel]>>h-1)&1)
		{
			fill_Buffer(ActiveBuffer, pixel, ColorMask); 
		}   
	}
	for (int pixel = 66 ; pixel < 110 ; pixel++)          // Fill in MINUTE pixels
	{
		if ((MinuteMask[pixel-66]>>FithOfMin-1)&1)
		{
			fill_Buffer(ActiveBuffer, pixel, ColorMask); 
		}  
	}
}

void fill_Buffer(int BufferID, int PixelID, uint32_t Color)
{
	uint32_t OutBuffer = BufferID ? OutBuffer_1 : OutBuffer_0;
	if (CurrentState == HSET || CurrentState == MSET)
	{
		Color = Color*Blink;
	}
	OutBuffer[PixelID] = Color;
}

uint32_t get_Buffer(int BufferID, int PixelID)
{
	uint32_t OutBuffer = BufferID ? OutBuffer_1 : OutBuffer_0;
	return OutBuffer[PixelID];
}

void clear_Buffer(int BufferID)
{
	uint32_t OutBuffer = BufferID ? OutBuffer_1 : OutBuffer_0;
	for (int Pixel = 0; Pixel<110; Pixel++)
	{
		OutBuffer[Pixel] = 0;
	}
}

void clearStrip()
{
	for(uint16_t i=0; i<strip.numPixels(); i++) 
	{
		strip.setPixelColor(i, 0);
	}
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
	if(WheelPos < 85) {
		return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	} 
	else if(WheelPos < 170) {
		WheelPos -= 85;
		return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	} 
	else {
		WheelPos -= 170;
		return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
	for(uint16_t i=0; i<strip.numPixels(); i++) {
		strip.setPixelColor(i, c);
		strip.show();
		delay2(wait);
	}
}

void rainbow(uint8_t wait) {
	uint16_t i, j;

	for(j=0; j<256; j++) {
		for(i=0; i<strip.numPixels(); i++) {
			strip.setPixelColor(i, Wheel((i+j) & 255));
		}
		strip.show();
		delay2(wait);
	}
}

void writeFrame(int FrameID)
{
  uint8_t x, y, r, g, b, pixel, brightness;
  clearStrip();
  switch (FrameID)
  {
	case 0:
	break;

	case H_FRAME:
	for (int i = 0; i<(sizeof(FrameH)/5);i++)
	{
		x = FrameH[i][X];
		y = FrameH[i][Y];
		r = FrameH[i][R];
		g = FrameH[i][G];
		b = FrameH[i][B];
		if (y%2)
		{
			pixel = x+11*y;
		}
		else
		{
			pixel = 10-x+11*y;
		}
		strip.setPixelColor(pixel, strip.Color(r, g, b));
	}
	break;

	case M_FRAME:
	for (int i = 0; i<(sizeof(FrameM)/5);i++)
	{
		x = FrameM[i][X];
		y = 9-FrameM[i][Y];
		r = FrameM[i][R];
		g = FrameM[i][G];
		b = FrameM[i][B];
		if (y%2)
		{
			pixel = x+11*y;
		}
		else
		{
			pixel = 10-x+11*y;
		}
		strip.setPixelColor(pixel, strip.Color(r, g, b));
	}
	break;

	case HI_FRAME:
	for (brightness = 0 ; brightness < 255 ; brightness++)
	{ 
		for (int i = 0; i<(sizeof(FrameHI)/5) ; i++)
		{
			x = 10-FrameHI[i][X];
			y = 9-FrameHI[i][Y];
			r = FrameHI[i][R];
			g = FrameHI[i][G];
			b = FrameHI[i][B];
			if (y%2)
			{
				pixel = x+11*y;
			}
			else
			{
				pixel = 10-x+11*y;
			}
			strip.setPixelColor(pixel, strip.Color(brightness, brightness, brightness));
		}
		strip.show();
		delay2(20);
	}
	break;

	case BYE_FRAME:
	for (brightness = 255 ; brightness > 0 ; brightness--)
	{ 
		for (int i = 0; i<(sizeof(FrameBYE)/5);i++)
		{
			x = 10-FrameBYE[i][X];
			y = 9-FrameBYE[i][Y];
			r = FrameBYE[i][R];
			g = FrameBYE[i][G];
			b = FrameBYE[i][B];
			if (y%2)
			{
				pixel = x+11*y;
			}
			else
			{
				pixel = 10-x+11*y;
			}
			strip.setPixelColor(pixel, strip.Color(brightness, brightness, brightness));
		}
		strip.show();
		delay2(20);
	}
	break;
	
	case RTC_FRAME:
	//display "RTC" on strip to indicate 
	break;
  }
  strip.show();
}
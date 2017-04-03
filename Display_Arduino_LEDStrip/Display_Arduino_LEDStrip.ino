/*
*/
#include <Arduino.h>

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

//#define DebugSerial Serial

#include "FastLED.h"

int Count_Val;
int Count_Init;
uint32_t prev_time;

bool Reset = false;
bool Stop = true;

void setup(void) {
	Serial.begin(115200);
  Stop = true;
  Count_Val = 240;
  Count_Init = 240;
  UpdateDisplay();

  LEDStrip_setup();
  
  prev_time = millis();
}

void loop(void) {
  UpdateDisplay();
  
	my_delay_ms(100);
}

/*///////////////////////////////////////////////////////////////////////////*/

void UpdateDisplay(void)
{
  #ifdef DebugSerial
  //DebugSerial.print("Count = ");
  //DebugSerial.println(Count_Val);
  #endif
  setSegments((Count_Val+9)/10);
}

/*///////////////////////////////////////////////////////////////////////////*/

void UpdateTime(void)
{
  if (!Stop)
  {
    uint32_t current = millis();
    int32_t interval = current - prev_time;
    if (interval>=100L)
    {
      prev_time = current + interval - 100;  
      if (Count_Val>1)
      {
        Count_Val--;
      }
      else
      {
        Count_Val = Count_Init;
        StopCount(true);
      }
    }
  }
}

String Count_str;

void ProcessCommand(char cmd) 
{
  if (Reset)
  {
    if ((cmd>='0')&&(cmd<='9'))
    {
      Count_str += cmd;
      if (Count_str.length()>=2)
      {
        Count_Init = Count_str.toInt()*10;
        Count_Val = Count_Init;
        Reset = false;
      }
      return;
    }
    else
    if (Count_str.length()>0)
    {
      Count_Init = Count_str.toInt()*10;
      Count_Val = Count_Init;
      Reset = false;
    }
    else
    {
      Count_Val = Count_Init;
      Reset = false;
    }
  }
  
  switch (cmd) {
    case 'Z':                     //start send Z
    case 'z':
      Reset = false;
      StopCount(false);
      break;
    case 'X':                     //stop send X
    case 'x':                     //stop send X
      Reset = false;
      StopCount(true);
      break;
    case 'Q':               //to reset, sample send Q14
    case 'q':               //to reset, sample send Q14
      Stop = true;
      Reset = true;
      Count_str = "";
      StopCount(true);
      break;
    case 'A':
    case 'a':
      Stop = true;
      Reset = false;
      Count_Val = Count_Init;
      StopCount(true);
      break;
  }
}

void my_delay_ms(int msec)
{
	uint32_t delay_val = msec;
	uint32_t endWait = millis();
	uint32_t beginWait = endWait;
	while (endWait - beginWait < delay_val)
	{
    UpdateTime();
    LEDStrip_loop();
		endWait = millis();
	}
}

void StopCount(bool bStop)
{
  if (!bStop)
  {
    Stop = false;
    if (Count_Val<=0) Count_Val = Count_Init;
    prev_time = millis();
  }
  else
  {
    Stop = true;    
  }
}



/*
*/
#include <Arduino.h>

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#include <SoftwareSerial.h>

SoftwareSerial ESPSerial(2,3);

//#define DebugSerial Serial
#define LED_ON  digitalWrite(LED_BUILTIN, HIGH)
#define LED_OFF digitalWrite(LED_BUILTIN, LOW)

int Count_Val;
int Count_Init;
uint32_t prev_time;

bool Reset = false;
bool Stop = true;
bool bEcho = false;

void setup(void) {
	Serial.begin(19200);
  ESPSerial.begin(19200);
  pinMode(2,INPUT_PULLUP);
  
  pinMode(LED_BUILTIN, OUTPUT);
  LED_ON;
  
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
      prev_time = current + 100L - interval;  
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

void serialServer_loop(void)
{
  while (Serial.available()>0) {
    unsigned char b = Serial.read();
    if (bEcho) Serial.write(b);  //echo
    ProcessCommand(b);
  }
  while (ESPSerial.available()>0) {
    unsigned char b = ESPSerial.read();
    ProcessCommand(b);
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
      if (Count_str.length()<2) return;
      
      Count_Init = Count_str.toInt()*10;
      Count_Val = Count_Init;
    }
    else
    if (Count_str.length()>0)
    {
      Count_Init = Count_str.toInt()*10;
      Count_Val = Count_Init;
    }
    else
    {
      Count_Val = Count_Init;
    }
    Reset = false;
    StopCount(true);
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
    case 'E':
    case 'e':
      bEcho = true;
      break;
    case 'F':
    case 'f':
      bEcho = false;
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
    serialServer_loop();
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
    LED_OFF;
  }
  else
  {
    Stop = true;
    LED_ON;
  }
}



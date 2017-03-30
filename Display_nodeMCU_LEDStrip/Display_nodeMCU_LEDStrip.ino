/*
*/
#include <Arduino.h>

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#include <ESP8266WiFi.h>
#include <WiFiServer.h>

unsigned int localPort = 1234;

WiFiServer wifiserver(localPort);

//#define DebugSerial Serial

int Count_Val;
int Count_Init;
uint32_t prev_time;
extern void (*DisplayLogo)(bool bLogo);

bool Reset = false;
bool Stop = true;

void setup(void) {
	Serial.begin(115200);
  Stop = true;
  Count_Val = 100;
  Count_Init = 100;

	for (int i = 0; i<400; i++)
	{
		if (WiFi.status() == WL_CONNECTED)
		{
			Serial.println(F("WiFi connected"));
			Serial.println(F("IP address: "));
			Serial.println(WiFi.localIP());
			break;
		}
		delay(50);
	}

	Serial.println(F("Starting WifiServer"));
	Serial.print(F("Local port: "));
	Serial.println(localPort);

  DisplayLogo = StartCount;
	webserver_setup();
  wifiserver.begin();

  prev_time = millis();

  LEDStrip_setup();
}

void loop(void) {
  UpdateDisplay();
  
	my_delay_ms(100);
}

/*///////////////////////////////////////////////////////////////////////////*/

void UpdateDisplay(void)
{
  #ifdef DebugSerial
  DebugSerial.print("Count = ");
  DebugSerial.println(Count_Val);
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
        Stop = true;
      }
    }
  }
}

WiFiClient client;

void wifiserver_loop(void)
{
  if (!client.connected())
  {
    client = wifiserver.available();
  }
  else
  {
    while (client.available()>0) {
      unsigned char b = client.read();
      client.write(b);
      Serial.write(b);
      ProcessCommand(b);
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
      StartCount(true);
      Reset = false;
      break;
    case 'X':                     //stop send X
    case 'x':                     //stop send X
      Stop = true;
      Reset = false;
      break;
    case 'Q':               //to reset, sample send Q14
    case 'q':               //to reset, sample send Q14
      Stop = true;
      Reset = true;
      Count_str = "";
      break;
    case 'A':
    case 'a':
      Stop = true;
      Reset = false;
      Count_Val = Count_Init;
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
		webserver_loop();
    wifiserver_loop();
		endWait = millis();
	}
}

void StartCount(bool bStart)
{
  if (bStart)
  {
    if (Count_Val<=0) Count_Val = Count_Init;
    Stop = false;
    prev_time = millis();
  }
  else
  {
    Stop = true;    
  }
}



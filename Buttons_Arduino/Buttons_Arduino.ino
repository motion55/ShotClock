
/*
  The circuit:
   pushbutton attached from pin 5 to +5V
   10K resistor attached from pin 5 to ground

   pushbutton attached from pin 6 to ground
   pushbutton attached from pin 7 to ground
*/
const int StartStop = 5;  //Start og Stop  See instruction above
const int Reset14 = 6;    //reset sa  14   i  sa ground
const int Reset24 = 7;     //reset sa 24    i conect sa ground
const int ResetENA = 8;
const int StartStopENA = 9;
const int StartStopLED = 10;

int Count_Val;
int Count_Init;
uint32_t prev_time;

bool Stop = true;

int buttonState = LOW;
unsigned long lastDebounceTime = 0;                             
unsigned long debounceDelay = 50;                               

//--------------------------------------------------
#include <SoftwareSerial.h>

SoftwareSerial ESPSerial(2,3);

#define ESP8266 Serial
#define LED_ON  digitalWrite(LED_BUILTIN, HIGH)
#define LED_OFF digitalWrite(LED_BUILTIN, LOW)

//--------------------------------------------------

void setup() {
  ESP8266.begin(19200);
  ESPSerial.begin(19200);
  pinMode(2, INPUT_PULLUP);
  
  setUpWifiShield();
  
  pinMode(StartStop, INPUT);
  pinMode(Reset14, INPUT_PULLUP);   
  pinMode(Reset24, INPUT_PULLUP); 
  pinMode(LED_BUILTIN, OUTPUT);
  LED_ON;
  
  Stop = true;
  Count_Val = 240;
  Count_Init = 240;
}

void loop() {
  ShotClockTriggeringCodes(); 
  UpdateTime();
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

//-----------------------------------------------------------------wifi CODES start

void setUpWifiShield() {
  pinMode(ResetENA, OUTPUT);
  pinMode(StartStopENA, OUTPUT);
  pinMode(StartStopLED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(ResetENA, LOW);
  digitalWrite(StartStopENA, HIGH);  
  digitalWrite(StartStopLED, HIGH);  
  digitalWrite(LED_BUILTIN, LOW);  
}

void SendTrytoSEND(String CommandData) 
{
  for (int i=0; i<CommandData.length(); i++)
  {
    uint8_t b = CommandData[i];
    ESP8266.write(b);
    ESPSerial.write(b);
  }
}

void ShotClockTriggeringCodes() {

  int readingStartStop = digitalRead(StartStop);
  int readingReset14 = digitalRead(Reset14);
  int readingReset24 = digitalRead(Reset24);

  //--------------------------------debounce start-------Start  Stop  button--------
  unsigned long CurrentTime = millis();                      
  
  if ((CurrentTime - lastDebounceTime) > debounceDelay) 
  { 
    if (readingStartStop != buttonState) 
    {
      buttonState = readingStartStop;
      if (buttonState == HIGH) 
      {
        if (Stop) 
        {
          StopCount(false);
          SendTrytoSEND("ZZ");
        }
        else
        {
          StopCount(true);
          SendTrytoSEND("XX");
        }
      }
      lastDebounceTime = CurrentTime;
    }
  }
  
  //---------------------------------debounce end-------Start  Stop  Button-------

  //-------------------------------------------------------reset14 and reset24 START------------------
  if (readingReset14 == LOW) 
  {
    StopCount(true);
    SendTrytoSEND("Q14");             //to reset 14
    delay(100);
  }
  if (readingReset24 == LOW) {
    StopCount(true);
    SendTrytoSEND("Q24");               //to reset 24
    delay(100);
  }
  //-------------------------------------------------------reset14 and reset24 END------------------

}



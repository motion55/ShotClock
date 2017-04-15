
/*
  The circuit:
    pushbutton on pin 5 to GND with 10K pullup to +3.3V
    pushbutton on pin 6 to GND with 10K pullup to +3.3V
    pushbutton on pin 7 to GND with 10K pullup to +3.3V
*/
const int StartStop = D5;  //Start og Stop  See instruction above
const int Reset14 = D6;    //reset sa  14   i  sa ground
const int Reset24 = D7;    //reset sa 24    i conect sa ground

int Count_Val;
int Count_Init;
uint32_t prev_time;

bool Stop = true;                                              

int buttonState = LOW;
unsigned long lastDebounceTime = 0;                             
unsigned long debounceDelay = 50;                               

#define _USE_UDP_  0
#define _USE_TCP_  1

//--------------------------------------------------

#define ESPSerial Serial
#define LED_ON  digitalWrite(LED_BUILTIN, LOW)
#define LED_OFF digitalWrite(LED_BUILTIN, HIGH)

//--------------------------------------------------

void setup() {
  ESPSerial.begin(19200);
  
  setUpWifiShield();
  
  pinMode(StartStop, INPUT_PULLUP);
  pinMode(Reset14, INPUT_PULLUP);
  pinMode(Reset24, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  LED_OFF;
  
  Stop = true;
  Count_Val = 240;
  Count_Init = 240;
}

void loop() {
  ShotClockTriggeringCodes(); 
  UpdateTime();
  Server_loop();
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

void StopCount(bool bStop)
{
  if (!bStop)
  {
    Stop = false;
    if (Count_Val<=0) Count_Val = Count_Init;
    prev_time = millis();
    LED_ON;
  }
  else
  {
    Stop = true;
    LED_OFF;
  }
}

//-----------------------------------------------------------------wifi CODES start

void setUpWifiShield() 
{
  Server_setup();
}

void SendTrytoSEND(String CommandData) 
{
  ESPSerial.write((const uint8_t*)CommandData.c_str(), CommandData.length());
#if _USE_UDP_  
  Send2UDPStr((const uint8_t*)CommandData.c_str(), CommandData.length());
#endif  
#if _USE_TCP_  
  Send2ClientStr((const uint8_t*)CommandData.c_str(), CommandData.length());
#endif  
}

void ShotClockTriggeringCodes() 
{
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
      if (buttonState == LOW)
      {
        if (Stop) 
        {
          StopCount(false);
          SendTrytoSEND("ZZZZZZZZZZZZZZZZ\r\n");
        }
        else
        {
          StopCount(true);
          SendTrytoSEND("XXXXXXXXXXXXXXXX\r\n");
        }
        delay(50);
      }
      lastDebounceTime = CurrentTime;
    }
  }

  //-------------------------------------------------------reset14 and reset24 START------------------
  if (readingReset14 == LOW) 
  {
    Count_Val = 140;
    Count_Init = 140;
    StopCount(true);
    SendTrytoSEND("Q14");             //to reset 14
    delay(100);
  }
  if (readingReset24 == LOW) 
  {
    Count_Val = 240;
    Count_Init = 240;
    StopCount(true);
    SendTrytoSEND("Q24");               //to reset 24
    delay(100);
  }
  //-------------------------------------------------------reset14 and reset24 END------------------
}




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
const int StartStopLED = 10;

int Na_set_na_Time = 24;
bool StartStopCondi = false;
bool Start = true;                                              
//bool Reset = false;
//bool Stop = true;

int buttonState = LOW;
unsigned long lastDebounceTime = 0;                             
unsigned long debounceDelay = 50;                               

//--------------------------------------------------
#include <SoftwareSerial.h>

SoftwareSerial ESPSerial(2,3);

#define ESP8266 Serial

//--------------------------------------------------

void setup() {
  ESP8266.begin(19200);
  ESPSerial.begin(19200);
  pinMode(2, INPUT_PULLUP);
  
  setUpWifiShield();
  
  pinMode(StartStop, INPUT);
  pinMode(Reset14, INPUT_PULLUP);   
  pinMode(Reset24, INPUT_PULLUP); 
  
  pinMode(StartStopLED, OUTPUT);
  digitalWrite(StartStopLED, LOW);  
}

void loop() {
  ShotClockTriggeringCodes(); 
}

//-----------------------------------------------------------------wifi CODES start

void setUpWifiShield() {
  digitalWrite(StartStopLED, HIGH);
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
        StartStopCondi = !StartStopCondi;
      }
      lastDebounceTime = CurrentTime;
    }
  }

  if (StartStopCondi) 
  {
    // Start condition
    if (Start) 
    {
      SendTrytoSEND("ZZ");
      Start = !Start;                                                        
    }
  } 
  else 
  {
    //Stop condition
    if (!Start) 
    {
      SendTrytoSEND("XX");
      Start = !Start;
    }
  }
  
  //---------------------------------debounce end-------Start  Stop  Button-------

  //-------------------------------------------------------reset14 and reset24 START------------------
  if (readingReset14 == LOW) {
    SendTrytoSEND("Q14");             //to reset 14
    delay(100);
  }
  if (readingReset24 == LOW) {
    SendTrytoSEND("Q24");               //to reset 24
    delay(100);
  }
  //-------------------------------------------------------reset14 and reset24 END------------------

}



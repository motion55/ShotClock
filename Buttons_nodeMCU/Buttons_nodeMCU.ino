
/*
  The circuit:
    pushbutton on pin 5 to GND with 10K pullup to +3.3V
    pushbutton on pin 6 to GND with 10K pullup to +3.3V
    pushbutton on pin 7 to GND with 10K pullup to +3.3V
*/
const int StartStop = D5;  //Start og Stop  See instruction above
const int Reset14 = D6;    //reset sa  14   i  sa ground
const int Reset24 = D7;    //reset sa 24    i conect sa ground

int Na_set_na_Time = 24;
bool StartStopCondi = false;
bool Start = true;                                              

int buttonState = LOW;
unsigned long lastDebounceTime = 0;                             
unsigned long debounceDelay = 50;                               

//--------------------------------------------------
#include <SoftwareSerial.h>

#define ESP8266 Serial

//--------------------------------------------------

void setup() {
  ESP8266.begin(19200);
  
  setUpWifiShield();
  
  pinMode(StartStop, INPUT_PULLUP);
  pinMode(Reset14, INPUT_PULLUP);
  pinMode(Reset24, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  
}

void loop() {
  ShotClockTriggeringCodes(); 
  Server_loop();
}

//-----------------------------------------------------------------wifi CODES start

void setUpWifiShield() {
  Server_setup();
}

void SendTrytoSEND(String CommandData) 
{
  for (int i=0; i<CommandData.length(); i++)
  {
    uint8_t b = CommandData[i];
    ESP8266.write(b);
    Send2Clients(b);
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
      if (buttonState == LOW)
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
      digitalWrite(LED_BUILTIN, HIGH);  
    }
  } 
  else 
  {
    //Stop condition
    if (!Start) 
    {
      SendTrytoSEND("XX");
      Start = !Start;
      digitalWrite(LED_BUILTIN, LOW);  
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



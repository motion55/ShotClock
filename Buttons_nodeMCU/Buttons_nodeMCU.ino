
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
bool Stop = true;                                              

int buttonState = LOW;
unsigned long lastDebounceTime = 0;                             
unsigned long debounceDelay = 50;                               

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
  Send2UDPStr((const uint8_t*)CommandData.c_str(), CommandData.length());
  Send2ClientStr((const uint8_t*)CommandData.c_str(), CommandData.length());
  for (int i=0; i<CommandData.length(); i++)
  {
    uint8_t b = CommandData[i];
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
      if (buttonState == LOW)
      {
        if (Stop) 
        {
          Stop = false;
          LED_ON;
          SendTrytoSEND("ZZ");
        }
        else
        {
          Stop = true;
          LED_OFF;
          SendTrytoSEND("XX");
        }
      }
      lastDebounceTime = CurrentTime;
    }
  }

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



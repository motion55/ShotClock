
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

int Na_set_na_Time = 24;
bool start = true;                                              
bool StartStopCondi = false;
bool reset = false;
bool Stop = true;

int buttonState;                    
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;                             
unsigned long debounceDelay = 50;                               

//-----------------------------------------------wifi CODES start
#define ESP8266 Serial
String ssid = "Thesis";
String password = "12345678";

//--------------------------------------------------wifi CODES end

void setup() {

  ESP8266.begin(19200);
  setUpWifiShield();
  pinMode(10, OUTPUT);
  pinMode(StartStop, INPUT);
  pinMode(Reset14, INPUT_PULLUP);   
  pinMode(Reset24, INPUT_PULLUP); 
  digitalWrite(StartStop, LOW);  
}

void loop() {
  ShotClockTriggeringCodes(); 

}

//-----------------------------------------------------------------wifi CODES start

void setUpWifiShield() {

  //delay(3000);
    ESP8266.println("AT+CWMODE=3");
 // if ( waitESP8266responseTRY()) {
    delay(1000);
    ESP8266.println("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"");
    delay(15000);
  //}
  //if ( waitESP8266responseTRY()) {
    ESP8266.println("AT+CIPMUX=1");
    delay(1000);
  //}
  //if ( waitESP8266responseTRY()) {
    ESP8266.println("AT+CIPSERVER=1,1234");
  delay(1000);
  //}
  //if ( waitESP8266responseTRY()) {
    ESP8266.println("AT+CIPSTO=28800");
  delay(1000);
  

  digitalWrite(10, HIGH);

}
bool waitESP8266responseTRY() {                                           
  while (ESP8266.available()) {                                 
    String readData = ESP8266.readStringUntil('\n');                      
    if (readData == "OK\r" || readData == "SEND OK\r") {                  
      ESP8266.println(readData);                                          
      return true;                                                        
      break;
    } else {
      return false;                                                       
      break;
    }
  } 
}
void SendTrytoSEND(String CommandData) {                                  
  for (int z = 0; z < 2; z++) {                 
    ESP8266.print("AT+CIPSEND=");
    ESP8266.print(z);


    
    ESP8266.print(",");
    ESP8266.println(CommandData.length());
    delay(150);
    //if ( waitESP8266responseTRY()) {
      ESP8266.println(CommandData);
    delay(150);
   // }
  }
}

void ShotClockTriggeringCodes() {

  int readingStartStop = digitalRead(StartStop);
  int readingReset14 = digitalRead(Reset14);
  int readingReset24 = digitalRead(Reset24);

  //--------------------------------debounce start-------Start  Stop  button--------
  if (readingStartStop != lastButtonState) {
    lastDebounceTime = millis();                      
  }
  if ((millis() - lastDebounceTime) > debounceDelay) { 
    if (readingStartStop != buttonState) {
      buttonState = readingStartStop;
      if (buttonState == HIGH) {
        StartStopCondi = !StartStopCondi;                                     
      }
    }
  }

  if (StartStopCondi) {
    //       // Start condition
    if (start) {
      SendTrytoSEND("ZZ");
      start = !start;                                                        
    }
  } else {                //Stop condition
    if (!start) {
      SendTrytoSEND("XX");
      start = !start;
    }
  }
  lastButtonState = readingStartStop;
  //---------------------------------debounce end-------Start  Stop  Button-------

  //-------------------------------------------------------reset14 and reset24 START------------------
  if (readingReset14 == LOW) {
    SendTrytoSEND("Q14");             //to reset 14
  }
  if (readingReset24 == LOW) {
    SendTrytoSEND("Q24");               //to reset 24

  }
  //-------------------------------------------------------reset14 and reset24 END------------------

}



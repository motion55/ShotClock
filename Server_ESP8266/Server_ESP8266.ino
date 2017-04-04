/*
*/
#include <Arduino.h>

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#define DebugSerial Serial

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Access point credentials
const char *ap_ssid = "Controller";
const char *ap_password = "12345678";

unsigned int localPort = 1234;
WiFiServer  wifiServer(localPort);

void setup(void) {
  Serial.begin(19200);
  
  IPAddress local_IP(192,168,6,1);
  IPAddress gateway(192,168,6,1);
  IPAddress subnet(255,255,255,0);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ap_ssid,ap_password); 

  webserver_setup();
}

void loop(void) {
  webserver_loop();
  wifiServer_loop();
  delay(10);
}

WiFiClient wifiClient;

void wifiServer_loop(void)
{
  if (wifiClient.connected())
  {
    while (Serial.available()>0) {
      uint8_t b = Serial.read();
      wifiClient.write(b);
    }
    while (wifiClient.available()>0) {
      uint8_t b = wifiClient.read();
      Serial.write(b);
    }
  }
  else
  {
    if ( wifiServer.hasClient())
    {
      wifiClient = wifiServer.available();
    }
    else
    {
      //wifiClient.stop();
    }
  }
}


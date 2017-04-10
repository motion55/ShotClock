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
WiFiServer wifiServer(localPort);

#define MAX_SRV_CLIENTS 2
WiFiClient wifiClients[MAX_SRV_CLIENTS];

void Server_setup(void) {
  IPAddress local_IP(192,168,6,1);
  IPAddress gateway(192,168,6,1);
  IPAddress subnet(255,255,255,0);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ap_ssid,ap_password); 

  webserver_setup();

  wifiServer.begin();
  wifiServer.setNoDelay(true);
}

void Server_loop(void) {
  webserver_loop();
  wifiServer_loop();
	delay(10);
}

void wifiServer_loop(void)
{
  uint8_t i;
  //check if there are any new clients
  if (wifiServer.hasClient())
  {
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!wifiClients[i] || !wifiClients[i].connected())
      {
        if(wifiClients[i]) wifiClients[i].stop();
        wifiClients[i] = wifiServer.available();
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = wifiServer.available();
    serverClient.stop();
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (wifiClients[i] && wifiClients[i].connected())
    {
      if(wifiClients[i].available())
      {
        //get data from the telnet client and push it to the UART
        while(wifiClients[i].available()) Serial.write(wifiClients[i].read());
      }
    }
  }
  //check UART for data
  if(Serial.available())
  {
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf,len);
    //push UART data to all connected telnet clients
    for(i = 0; i < MAX_SRV_CLIENTS; i++)
    {
      if (wifiClients[i] && wifiClients[i].connected())
      {
        wifiClients[i].write(sbuf,len);
        delay(1);
      }
    }
  }
}

void Send2Clients(String DataStr)
{
  for(uint8_t i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (wifiClients[i] && wifiClients[i].connected())
    {
      for (uint8_t j = 0; j < DataStr.length(); j++)
      {
        uint8_t dat = DataStr[j];
        wifiClients[i].write(dat);
      }
      delay(1);
    }
  }
}


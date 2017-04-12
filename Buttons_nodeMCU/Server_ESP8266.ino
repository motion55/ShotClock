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
#include <WiFiUdp.h>

// Access point credentials
const char *ap_ssid = "Controller";
const char *ap_password = "12345678";

String sta_ssid;
String sta_passwd;

unsigned int localPort = 1234;
WiFiServer wifiServer(localPort);

#define MAX_SRV_CLIENTS 2
WiFiClient wifiClients[MAX_SRV_CLIENTS];

#define WIFI_CONNECT_INTERVAL   5000L
#define MAX_RECONNECT 2

int Reconnect;
unsigned long last_connect;
unsigned long interval = WIFI_CONNECT_INTERVAL;
bool bWiFiConnect;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

void Server_setup(void) {
  IPAddress local_IP(192,168,6,1);
  IPAddress gateway(192,168,6,1);
  IPAddress subnet(255,255,255,0);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ap_ssid,ap_password); 

  if (WiFi.SSID().length()>0)
  {
    sta_ssid = WiFi.SSID();
    sta_passwd = WiFi.psk();
  }
#ifdef DebugSerial
  DebugSerial.println();
  DebugSerial.println("ShotClock initializing...");
  DebugSerial.print("Connecting to SSID:");
  DebugSerial.print(WiFi.SSID());
  DebugSerial.print("-Password:");
  DebugSerial.println(WiFi.psk());
#endif  
  webserver_setup();
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
    for(i = 0; i < MAX_SRV_CLIENTS; i++)
    {
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
        while(wifiClients[i].available()) ESPSerial.write(wifiClients[i].read());
      }
    }
  }
  //check UART for data
  size_t len = ESPSerial.available();
  if(len>0)
  {
    uint8_t sbuf[len];
    ESPSerial.readBytes(sbuf,len);
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

void Send2Clients(uint8_t dat)
{
  for(uint8_t i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (wifiClients[i] && wifiClients[i].connected())
    {
      wifiClients[i].write(dat);
      delay(1);
    }
  }
}

void Send2ClientStr(String DataStr)
{
  for(uint8_t i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (wifiClients[i] && wifiClients[i].connected())
    {
      wifiClients[i].write(DataStr.c_str(), DataStr.length());
      delay(1);
    }
  }
}

void Send2UDPStr(String DataStr)
{
  if (WiFi.isConnected())
  {
    IPAddress address = WiFi.localIP();
    address[3] = 0xFF;
    udp.beginPacket(address, localPort);
    udp.write(DataStr.c_str(), DataStr.length());
    udp.endPacket();
  }
}


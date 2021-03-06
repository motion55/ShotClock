/*
*/
#include <Arduino.h>

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

//#define DebugSerial Serial

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

// Access point credentials
String ap_ssid("ShotClock");
const char *ap_password = "12345678";

String sta_ssid("Controller");
String sta_passwd("12345678");

IPAddress serverIP(192,168,6,1);
uint16_t localPort = 1234;

#define SERVER_CONNECT_INTERVAL 2000L
#define WIFI_CONNECT_INTERVAL   5000L
#define MAX_RECONNECT 2

int Reconnect;
unsigned long last_connect;
unsigned long interval = WIFI_CONNECT_INTERVAL;
bool bServerConnect;
bool bWiFiConnect;

WiFiClient wifiClient;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

void setup(void) {
  Serial.begin(19200);
  
  IPAddress local_IP(192,168,5,1);
  IPAddress gateway(192,168,5,1);
  IPAddress subnet(255,255,255,0);

  WiFi.softAPConfig(local_IP, gateway, subnet);

  String macAddr = WiFi.softAPmacAddress();
  ap_ssid += '_' + macAddr.substring(12,14) + macAddr.substring(15);
  WiFi.softAP(ap_ssid.c_str(),ap_password); 
  
  if (WiFi.SSID().length()>0)
  {
    sta_ssid = WiFi.SSID();
    sta_passwd = WiFi.psk();
  }
#ifdef DebugSerial
  DebugSerial.println();
  DebugSerial.print(ap_ssid);
  DebugSerial.println(" initializing...");
  DebugSerial.print("Connecting to SSID:");
  DebugSerial.print(WiFi.SSID());
  DebugSerial.print("-Password:");
  DebugSerial.println(WiFi.psk());
#endif  
  webserver_setup();

  udp.begin(localPort);
#ifdef DebugSerial
  DebugSerial.print("Starting UDP ");
  DebugSerial.print("Local port: ");
  DebugSerial.println(udp.localPort());
#endif  
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  wifiClient.setNoDelay(true);
}

void loop(void) {
  webserver_loop();
  wifiClient_loop();
  delay(10);
}

void wifiClient_loop(void)
{
  unsigned long curr_connect = millis();
  bool bWiFiConnected = WiFi.isConnected();
  bool bClientConnected = wifiClient.connected();
  
  if (bWiFiConnected)
  {
    size_t cb = udp.parsePacket();
    if (cb>0)
    {
      byte packetBuffer[cb];
      udp.read(packetBuffer, cb);
      Serial.write(packetBuffer, cb);
    }
  }
  
  if (bWiFiConnected&&bClientConnected)
  {
    last_connect = curr_connect;
    Reconnect = 0;
    
    size_t len = Serial.available();
    if (len>0) 
    {
      uint8_t buf[len];
      Serial.readBytes(buf, len);
      wifiClient.write(buf, len);
    }

    len = wifiClient.available();
    if (len>0) 
    {
      uint8_t buf[len];
      wifiClient.read(buf, len);
      Serial.write(buf, len);
    }
  }
  else
  {
    if (curr_connect - last_connect >= interval)
    {
      last_connect = curr_connect;
      
      if (bWiFiConnected) 
      {
        if (bWiFiConnect)
        {
          #ifdef DebugSerial
          DebugSerial.print("WLAN Connected to ");
          DebugSerial.println(sta_ssid);
          #endif
          bWiFiConnect = false;
          if (WiFi.gatewayIP()==IPAddress(192,168,6,1))
          {
            serverIP = IPAddress(192,168,6,1);
          }
          if (WiFi.gatewayIP()==serverIP)
          {
            serverConnect(true);
            Reconnect = 0;
          }
          else
          {
            serverConnect(false);
            serverIP = WiFi.gatewayIP();
          }
        }
        else if (bServerConnect)
        {
          if (Reconnect<MAX_RECONNECT)
          {
            wifiClient.stop();
            serverConnect(true);
            Reconnect++;
            interval += SERVER_CONNECT_INTERVAL*Reconnect;
          }
        }
      }
      else 
      {
        interval = WIFI_CONNECT_INTERVAL;
        #ifdef DebugSerial
        DebugSerial.write('.');
        #endif
        if (Reconnect<MAX_RECONNECT)
        {
          #ifdef DebugSerial
          DebugSerial.print("Reconnect to SSID:");
          DebugSerial.print(sta_ssid);
          DebugSerial.print(" Password:");
          DebugSerial.println(sta_passwd);
          #endif
          Reconnect++;
          interval += WIFI_CONNECT_INTERVAL*Reconnect;
          WiFi.begin(sta_ssid.c_str(), sta_passwd.c_str());
        }
      }
    }
  }
}

void serverConnect(bool Connect)
{
  if (Connect)
  {
    bServerConnect = true;
    last_connect = millis();
    interval = SERVER_CONNECT_INTERVAL;
    wifiClient.connect(serverIP, localPort);
    #ifdef DebugSerial
    DebugSerial.print("Connecting to ");
    DebugSerial.print(serverIP.toString());
    DebugSerial.print(" Port No.:");
    DebugSerial.println(localPort);
    #endif
  }
  else
  {
    bServerConnect = false;
    wifiClient.stop();
    #ifdef DebugSerial
    DebugSerial.println("Disconnected from server.");
    #endif
  }
}



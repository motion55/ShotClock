
#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

ESP8266WebServer webserver(80);

void webserver_setup()
{
  /* Set page handler functions */
  webserver.on("/", rootPageHandler);
  webserver.on("/wlan_config", wlanPageHandler);
  webserver.on("/server", serverPageHandler);
  webserver.onNotFound(handleNotFound);

  webserver.begin();

  bWiFiConnect = true;
  bServerConnect = false;
  last_connect = millis();
  interval = WIFI_CONNECT_INTERVAL;
  Reconnect = 0;
}

inline void webserver_loop()
{
  webserver.handleClient();
}

/* Root page for the webserver */
void rootPageHandler()
{
  String response_message = "<html><head><title>ShotClock Display Client</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>ShotClock Display</center></h1>";
  response_message += "<h2><center>Controller Client</center></h3>";

  if (WiFi.isConnected())
  {
    response_message += "<h3><center>WLAN Status: Connected</center></h3>";
  }
  else
  {
    response_message += "<h3><center>WLAN Status: Disconnected</center></h3>";
  }

  response_message += "<h4><center><a href=\"/wlan_config\">Configure WLAN settings</a></center></h4>";
  response_message += "<h4><center><a href=\"/server\">Configure Server settings</a></center></h4>";
  response_message += "</body></html>";

  webserver.send(200, "text/html", response_message);
}

/* WLAN page allows users to set the WiFi credentials */
void wlanPageHandler()
{
  // Check if there are any GET parameters
  bool bChange = false;
  if (webserver.hasArg("ssid"))
  {
    if (sta_ssid!=webserver.arg("ssid"))
    {
      sta_ssid = webserver.arg("ssid");
      bChange = true;
    }
    if (webserver.hasArg("password"))
    {
      if (sta_passwd!=webserver.arg("password"))
      {
        sta_passwd = webserver.arg("password");
        bChange = true;
      }
    }
    else
    {
      if (sta_passwd.length()>0) 
      {
        sta_passwd = "";
        bChange = true;
      }
    }
    
    if ((!bWiFiConnect)||bChange)
    {
      if (bServerConnect) serverConnect(false);
      
      if (sta_passwd.length()>0)
      {
        WiFi.begin(sta_ssid.c_str(), sta_passwd.c_str());
      }
      else
      {
        WiFi.begin(sta_ssid.c_str());
      }
      #ifdef DebugSerial
      DebugSerial.print("Connecting to SSID:");
      DebugSerial.print(sta_ssid);
      DebugSerial.print(" Password:");
      DebugSerial.println(sta_passwd);
      #endif
      webserver_setup();
    }
  }

  String response_message = "";
  response_message += "<html>";
  response_message += "<head><title>ShotClock Display Client</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>WLAN Settings</center></h1>";
  
  response_message += "<center><a href=\"/\">Return to main page</a></center><br>";

  if (WiFi.isConnected())
  {
    response_message += "<center>WiFi Status: Connected</center>";
  }
  else
  {
    if (bWiFiConnect)
    {
      response_message += "<center>WiFi Status: Connecting</center>";
    }
    else
    {
      response_message += "<center>WiFi Status: Disconnected</center>";
    }
  }
  response_message += "<center>Local IP Address: "+WiFi.localIP().toString()+"</center>";
  response_message += "<center>Subnet Mask: "+WiFi.subnetMask().toString()+"</center>";
  response_message += "<center>GW IP Address: "+WiFi.gatewayIP().toString()+"</center><br>";

  response_message += "<center><p>To connect to a WiFi network, please select a network...</p></center>";

  // Get number of visible access points
  int ap_count = WiFi.scanNetworks();

  if (ap_count == 0)
  {
    response_message += "<center>No access points found.</center><br>";
  }
  else
  {
    response_message += "<form method=\"get\">";
    // Show access points
    for (uint8_t ap_idx = 0; ap_idx < ap_count; ap_idx++)
    {
      if (String(WiFi.SSID(ap_idx))==sta_ssid)
      {
        response_message += "<center><input type=\"radio\" name=\"ssid\" value=\"" + String(WiFi.SSID(ap_idx)) + "\" checked>";
      }
      else
      {
        response_message += "<center><input type=\"radio\" name=\"ssid\" value=\"" + String(WiFi.SSID(ap_idx)) + "\">";
      }
      response_message += String(WiFi.SSID(ap_idx)) + " (RSSI: " + WiFi.RSSI(ap_idx) + ")";
      (WiFi.encryptionType(ap_idx) == ENC_TYPE_NONE) ? response_message += " " : response_message += "*";
      response_message += "</center><br>";
    }

    response_message += "<center>WiFi password (if required):</center>";
    response_message += "<center><input type=\"text\" value=\""+sta_passwd+"\" name=\"password\"></center><br>";
    response_message += "<center><input type=\"submit\" value=\"Connect\"></center>";
    response_message += "</form>";
  }
  
  response_message += "</body></html>";

  webserver.send(200, "text/html", response_message);
}

/*///////////////////////////////////////////////////////////////////////////*/

void serverPageHandler()
{
  // Check if there are any GET parameters
  if (webserver.hasArg("serverIP"))
  {
    serverIP.fromString(webserver.arg("serverIP").c_str());
  }

  if (webserver.hasArg("portno"))
  {
    localPort = (uint16_t)webserver.arg("portno").toInt();
  }
  
  if (webserver.hasArg("action"))
  {
    if (webserver.arg("action") == "Connect")
    {
      serverConnect(true);
      Reconnect = 0;
    }
    else
    {
      serverConnect(false);
    }
  }

  String response_message = "<html><head><title>ShotClock Display Client</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>Configure Server</center></h1>";

  response_message += "<center><a href=\"/\">Return to main page</a></center><br>";

  bool bConnected;
  
  if (wifiClient.connected())
  {
    response_message += "<center>Status: Connected to server</center>";
    bConnected = true;
  }
  else
  {
    response_message += "<center>Status: Disconnected from server</center>";
    bConnected = false;
  }
  response_message += "<center>Local IP Address: "+WiFi.localIP().toString()+"</center>";
  response_message += "<center>Subnet Mask: "+WiFi.subnetMask().toString()+"</center>";
  response_message += "<center>GW IP Address: "+WiFi.gatewayIP().toString()+"</center><br>";

  response_message += "<form method=\"get\">";
  
  response_message += "<center>Server Address</center>";
  response_message += "<center><input type=\"text\" value=\""+serverIP.toString()+"\" name=\"serverIP\"></center><br>";
  
  response_message += "<center>Port No.</center>";
  response_message += "<center><input type=\"text\" value=\""+String(localPort)+"\" name=\"portno\"></center><br>";
  
  if (bConnected == false)
  {
    if (bWiFiConnect)
    {
      response_message += "<center><input type=\"submit\" name=\"action\" value=\"Connecting\"></center>";
    }
    else
    {
      response_message += "<center><input type=\"submit\" name=\"action\" value=\"Connect\"></center>";
    }
  }
  else
  {
    response_message += "<center><input type=\"submit\" name=\"action\" value=\"Disconnect\"></center>";
  }
  response_message += "</form>";
  
  response_message += "</body></html>";

  webserver.send(200, "text/html", response_message);
}

/* Called if requested page is not found */
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webserver.uri();
  message += "\nMethod: ";
  message += (webserver.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webserver.args();
  message += "\n";

  for (uint8_t i = 0; i < webserver.args(); i++)
  {
    message += " " + webserver.argName(i) + ": " + webserver.arg(i) + "\n";
  }

  webserver.send(404, "text/plain", message);
}


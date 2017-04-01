
#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

// Access point credentials
const char *ap_ssid = "Controller";
const char *ap_password = "12345678";

const char *server_ssid = "ShotClock";
const char *server_password = "12345678";

ESP8266WebServer webserver(80);

void webserver_setup()
{
#if 1
  IPAddress local_IP(192,168,6,1);
  IPAddress gateway(192,168,6,1);
  IPAddress subnet(255,255,255,0);

  WiFi.softAPConfig(local_IP, gateway, subnet);
#endif  
  WiFi.softAP(ap_ssid,ap_password); 
  /* Set page handler functions */
  webserver.on("/", rootPageHandler);
  webserver.on("/wlan_config", wlanPageHandler);
  webserver.on("/server", serverPageHandler);
  webserver.onNotFound(handleNotFound);

  webserver.begin();

  delay(1000);
  WiFi.begin(server_ssid,server_password);
  delay(3000);
}

inline void webserver_loop()
{
  webserver.handleClient();
}

/* Root page for the webserver */
void rootPageHandler()
{
  String response_message = "<html><head><title>ESP8266 Webserver</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>ESP8266 Webserver</center></h1>";

  if (WiFi.status() == WL_CONNECTED)
  {
    response_message += "<h3><center>WLAN Status: Connected</center></h3>";
  }
  else
  {
    response_message += "<h3><center>WLAN Status: Disconnected</center></h3>";
  }

  response_message += "<h4><center><a href=\"/wlan_config\">Configure WLAN settings</a></center></h4>";
  response_message += "<h4><center><a href=\"/server\">Configure Server</a></center></h4>";
  response_message += "</body></html>";

  webserver.send(200, "text/html", response_message);
}

/* WLAN page allows users to set the WiFi credentials */
void wlanPageHandler()
{
  String passwd_str;
  // Check if there are any GET parameters
  if (webserver.hasArg("ssid"))
  {
    if (webserver.hasArg("password"))
    {
      WiFi.begin(webserver.arg("ssid").c_str(), webserver.arg("password").c_str());
      passwd_str = webserver.arg("password");
      #ifdef DebugSerial
      DebugSerial.print("Connecting to ");
      DebugSerial.print(webserver.arg("ssid"));
      DebugSerial.print(" Password:");
      DebugSerial.println(passwd_str);
      #endif
    }
    else
    {
      #ifdef DebugSerial
      WiFi.begin(webserver.arg("ssid").c_str());
      DebugSerial.print("Connecting to ");
      DebugSerial.print(webserver.arg("ssid"));
      DebugSerial.print(" No Password.");
      #endif
    }

    for (int i = 0; i<200; i++)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        #ifdef DebugSerial
        DebugSerial.println("WiFi reconnected");
        DebugSerial.println("Local IP address: ");
        DebugSerial.println(WiFi.localIP());
        #endif
        serverIP = WiFi.gatewayIP();
        break;
      }
      delay(50);
    }
    delay(1000);
    webserver_setup();
  }

  String response_message = "";
  response_message += "<html>";
  response_message += "<head><title>WLAN Settings</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>WLAN Settings</center></h1>";
  
  response_message += "<center><a href=\"/\">Return to main page</a></center><br>";

  if (WiFi.status() == WL_CONNECTED)
  {
    response_message += "<center>Status: Connected</center>";
  }
  else
  {
    response_message += "<center>Status: Disconnected</center>";
  }
  response_message += "<center>Local IP Adress:"+WiFi.localIP().toString()+"</center>";
  response_message += "<center>Gateway IP Adress:"+WiFi.subnetMask().toString()+"</center>";
  response_message += "<center>Gateway IP Adress:"+WiFi.gatewayIP().toString()+"</center><br>";

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
      response_message += "<center><input type=\"radio\" name=\"ssid\" value=\"" + String(WiFi.SSID(ap_idx)) + "\">";
      response_message += String(WiFi.SSID(ap_idx)) + " (RSSI: " + WiFi.RSSI(ap_idx) + ")";
      (WiFi.encryptionType(ap_idx) == ENC_TYPE_NONE) ? response_message += " " : response_message += "*";
      response_message += "</center><br>";
    }

    response_message += "<center>WiFi password (if required):</center>";
    response_message += "<center><input type=\"text\" value=\""+passwd_str+"\" name=\"password\"></center><br>";
    response_message += "<center><input type=\"submit\" value=\"Connect\"></center>";
    response_message += "</form>";
  }
  
  response_message += "</body></html>";

  webserver.send(200, "text/html", response_message);
}


/*///////////////////////////////////////////////////////////////////////////*/
#define _USE_BUTTON_  1

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
  
#if _USE_BUTTON_
  if (webserver.hasArg("action"))
#else
  if (webserver.hasArg("gpio2"))
#endif    
  {
#if _USE_BUTTON_
    if (webserver.arg("action") == "Connect")
#else
    if (webserver.arg("gpio2") == "1")
#endif    
    {
      serverConnect(true);
    }
    else
    {
      serverConnect(false);
    }
  }

  String response_message = "<html><head><title>Configure Server</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>Configure Server</center></h1>";

  response_message += "<center><a href=\"/\">Return to main page</a></center><br>";

  bool Connected;
  
  if (wificlient.connected())
  {
    response_message += "<center>Status: Connected to server</center>";
    Connected = true;
  }
  else
  {
    response_message += "<center>Status: Disconnected from server</center>";
    Connected = false;
  }
  response_message += "<center>Local IP Adress:"+WiFi.localIP().toString()+"</center>";
  response_message += "<center>Gateway IP Adress:"+WiFi.subnetMask().toString()+"</center>";
  response_message += "<center>Gateway IP Adress:"+WiFi.gatewayIP().toString()+"</center><br>";

  response_message += "<form method=\"get\">";
  
  response_message += "<center>Server Address</center>";
  response_message += "<center><input type=\"text\" value=\""+serverIP.toString()+"\" name=\"serverIP\"></center><br>";
  
  response_message += "<center>Port No.</center>";
  response_message += "<center><input type=\"text\" value=\""+String(localPort)+"\" name=\"portno\"></center><br>";
  
#if (_USE_BUTTON_==0)
  response_message += "<center>Server Connect</center>";
#endif
  if (Connected == false)
  {
#if _USE_BUTTON_
    if (bConnect)
    {
      response_message += "<center><input type=\"submit\" name=\"action\" value=\"Connecting\"></center>";
    }
    else
    {
      response_message += "<center><input type=\"submit\" name=\"action\" value=\"Connect\"></center>";
    }
#else   
    response_message += "<center><input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\">On</center><br>";
    response_message += "<center><input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\" checked>Off</center><br>";
#endif    
  }
  else
  {
#if _USE_BUTTON_
    response_message += "<center><input type=\"submit\" name=\"action\" value=\"Disconnect\"></center>";
#else   
    response_message += "<center><input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\" checked>On</center><br>";
    response_message += "<center><input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\">Off</center><br>";
#endif    
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


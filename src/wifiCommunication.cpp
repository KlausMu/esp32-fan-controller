#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#endif
#if defined(ESP8266)
  #include <ESP8266WiFi.h> 
#endif

#include "config.h"
#include "log.h"
#include "wifiCommunication.h"

#ifdef useWIFI

boolean connected = false;
bool wifiIsDisabled = true;
String accessPointName;

void setAccessPointName() {
  String BSSID = String(WiFi.BSSIDstr());
  if        (BSSID == "00:11:22:33:44:55") {
      accessPointName = "Your AP 2,4 GHz";
  } else if (BSSID == "66:77:88:99:AA:BB") {
      accessPointName = "Your AP 5 GHz";
  }
}

#if defined(ESP32)
void printWiFiStatus(void){
  if (wifiIsDisabled) return;

  if (WiFi.isConnected()) {
    Serial.printf(MY_LOG_FORMAT("  WiFi.status() == connected\r\n"));
  } else {
    Serial.printf(MY_LOG_FORMAT("  WiFi.status() == DIS-connected\r\n"));
  }
  // Serial.println(WiFi.localIP());
  Serial.printf(MY_LOG_FORMAT("  IP address: %s\r\n"), WiFi.localIP().toString().c_str());

  if (WiFi.isConnected()) { //  && WiFi.localIP().isSet()) {
    Serial.printf(MY_LOG_FORMAT("  WiFi connected and IP is set :-)\r\n"));
  } else {
    Serial.printf(MY_LOG_FORMAT("  WiFi not completely available :-(\r\n"));
  }
}
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.printf(MY_LOG_FORMAT("  Callback \"StationConnected\"\r\n"));

  printWiFiStatus();
}
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.printf(MY_LOG_FORMAT("  Callback \"StationDisconnected\"\r\n"));
  connected = false;

  printWiFiStatus();

  // shouldn't even be here when wifiIsDisabled, but still happens ...
  if (!wifiIsDisabled) {
    WiFi.begin(wifi_ssid, wifi_password);
  }
}
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.printf(MY_LOG_FORMAT("  Callback \"GotIP\"\r\n"));
  connected = true;
  setAccessPointName();

  printWiFiStatus();
}
#endif
#if defined(ESP8266)
void printWiFiStatus(void){
  if (wifiIsDisabled) return;

  if (WiFi.isConnected()) {
    Serial.printf(MY_LOG_FORMAT("  WiFi.status() == connected\r\n"));
  } else {
    Serial.printf(MY_LOG_FORMAT("  WiFi.status() == DIS-connected\r\n"));
  }
  // Serial.println(WiFi.localIP());
  Serial.printf(MY_LOG_FORMAT("  IP address: %s\r\n"), WiFi.localIP().toString().c_str());

  if (WiFi.isConnected()) { //  && WiFi.localIP().isSet()) {
    Serial.printf(MY_LOG_FORMAT("  WiFi connected and IP is set :-)\r\n"));
  } else {
    Serial.printf(MY_LOG_FORMAT("  WiFi not completely available :-(\r\n"));
  }
}
//callback on WiFi connected
void onSTAConnected (WiFiEventStationModeConnected event_info) {
  Serial.printf(MY_LOG_FORMAT("  Callback \"onStationModeConnected\"\r\n"));
  Serial.printf(MY_LOG_FORMAT("  Connected to %s\r\n"), event_info.ssid.c_str ());

  printWiFiStatus();
}
// Manage network disconnection
void onSTADisconnected (WiFiEventStationModeDisconnected event_info) {
  Serial.printf(MY_LOG_FORMAT("  Callback \"onStationModeDisconnected\"\r\n"));
  Serial.printf(MY_LOG_FORMAT("  Disconnected from SSID: %s\r\n"), event_info.ssid.c_str ());
  Serial.printf(MY_LOG_FORMAT("  Reason: %d\r\n"), event_info.reason);
  connected = false;

  printWiFiStatus();

  // shouldn't even be here when wifiIsDisabled, but still happens ...
  if (!wifiIsDisabled) {
    WiFi.begin(wifi_ssid, wifi_password);
  }
}
//callback on got IP address
// Start NTP only after IP network is connected
void onSTAGotIP (WiFiEventStationModeGotIP event_info) {
  Serial.printf(MY_LOG_FORMAT("  Callback \"onStationModeGotIP\"\r\n"));
  Serial.printf(MY_LOG_FORMAT("  Got IP: %s\r\n"), event_info.ip.toString ().c_str ());
  Serial.printf(MY_LOG_FORMAT("  Connected: %s\r\n"), WiFi.isConnected() ? "yes" : "no");
  connected = true;
  setAccessPointName();

  printWiFiStatus();
}
#endif

void wifi_enable(void) {
  wifiIsDisabled = false;

  #if defined(ESP32)
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 2)
  WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  #else
  WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  #endif
  #endif
  #if defined(ESP8266)
  static WiFiEventHandler e2;
  e2 = WiFi.onStationModeDisconnected(onSTADisconnected);
  #endif
  WiFi.begin(wifi_ssid, wifi_password);
}
void wifi_disable(void){
  wifiIsDisabled = true;

  #if defined(ESP32)
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 2)
  WiFi.removeEvent(ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  #else
  WiFi.removeEvent(SYSTEM_EVENT_STA_DISCONNECTED);
  #endif
  #endif
  #if defined(ESP8266)
  // not tested
  WiFi.onStationModeDisconnected(NULL);
  #endif
  WiFi.disconnect();
}

void wifi_setup(){
/*  
  WiFi will be startetd here. Won't wait until WiFi has connected.
  Event connected:    Will only be logged, nothing else happens
  Event GotIP:        From here on WiFi can be used. Only from here on IP address is available
  Event Disconnected: Will automatically try to reconnect here. If reconnection happens, first event connected will be fired, after this event gotIP fires
*/
  #if defined(ESP32)
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 2)
  WiFi.onEvent(WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);    
  #else
  WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);    
  #endif
  #endif
  #if defined(ESP8266)
  static WiFiEventHandler e1, e2, e3;
  e1 = WiFi.onStationModeConnected(onSTAConnected);
  e2 = WiFi.onStationModeDisconnected(onSTADisconnected);
  e3 = WiFi.onStationModeGotIP(onSTAGotIP);// As soon WiFi is connected, start NTP Client
  #endif
  WiFi.mode(WIFI_STA);

  wifi_disable();
}
#endif
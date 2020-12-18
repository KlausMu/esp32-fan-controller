#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "log.h"

#ifdef useWIFI
bool checkWiFiAndReconnect(int halfSeconds);
bool checkWiFiAndReconnectTwice();
#endif

void setup_wifi(){
  #ifdef useWIFI
  delay(100);
  checkWiFiAndReconnectTwice();
  #else
  log_printf(MY_LOG_FORMAT("    WiFI is disabled in config.h"));
  #endif
}

#ifdef useWIFI
bool checkWiFiAndReconnectTwice(){
  bool result;
  result = checkWiFiAndReconnect(3);
  if (!result){
    result = checkWiFiAndReconnect(10);
  }
  return result;
}

bool checkWiFiAndReconnect(int halfSeconds){
  if ((WiFi.status() != WL_CONNECTED)) {
    log_printf(MY_LOG_FORMAT("  Connecting to %s"), wifi_ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);
    int counter=0;
    while ((WiFi.status() != WL_CONNECTED) && (counter < halfSeconds)) {
      delay(500);
      log_printf(MY_LOG_FORMAT("  ."));
      counter++;
    }
    if ((WiFi.status() == WL_CONNECTED)) {
      log_printf(MY_LOG_FORMAT("  WiFi connected to %s. IP address: %s"), wifi_ssid, WiFi.localIP().toString().c_str());
    } else {
      log_printf(MY_LOG_FORMAT("  WiFi connection failed."));
    }
  }

  return (WiFi.status() == WL_CONNECTED);
}
#endif

#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "log.h"
#include "wifiCommunication.h"

#ifdef showShutdownButton
HTTPClient http;

bool shutdownRaspPi(){
  bool res = false;
  if (checkWiFiAndReconnectTwice()){
    HTTPClient http;
    http.begin(shutdownRequest); // Specify the URL
    int httpCode = http.GET();  //   GET();   // Make the request
 
    if (httpCode > 0) { //Check for the returning code
        String payload = http.getString();
        log_printf(MY_LOG_FORMAT("httpCode = %d"), httpCode);
        log_printf(MY_LOG_FORMAT("payload = %s"), payload);
    } else {
      log_printf(MY_LOG_FORMAT("Cannot shutdown. Error on HTTP request"));
    }
    http.end(); //Free the resources
    res = (httpCode == 200);
  } else {
    log_printf(MY_LOG_FORMAT("Cannot shutdown. No WiFi connection."));
  }
  return res;
}
#endif
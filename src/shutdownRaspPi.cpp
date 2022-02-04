#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "log.h"
#include "wifiCommunication.h"

#ifdef showShutdownButton
HTTPClient http;

bool shutdownRaspPi(){
  if (wifiIsDisabled) {
    Log.printf("Cannot shutdown. No WiFi connection.\r\n");
    return false;
  }

  bool res = false;
  HTTPClient http;
  http.begin(shutdownRequest);
  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.POST(shutdownPayload);

  if (httpCode > 0) { //Check for the returning code
      String payload = http.getString();
      Log.printf("httpCode = %d\r\n", httpCode);
      Log.printf("payload = %s\r\n", payload.c_str());
  } else {
    Log.printf("Cannot shutdown. Error on HTTP request\r\n");
  }
  http.end(); //Free the resources
  res = (httpCode == 200);

  return res;
}
#endif
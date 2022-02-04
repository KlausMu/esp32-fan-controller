#include <Arduino.h>

#include "config.h"
#include "log.h"
#include "wifiCommunication.h"
#include "mqtt.h"
#include "sensorBME280.h"
#include "fanPWM.h"
#include "fanTacho.h"
#include "temperatureController.h"
#include "tft.h"
#include "tftTouch.h"

#if defined(useOTAUpdate)
  // https://github.com/SensorsIot/ESP32-OTA
  #include "OTA.h"
  #if !defined(useOTA_RTOS)
    #include <ArduinoOTA.h>
  #endif
#endif
#if defined(useTelnetStream)
#include "TelnetStream.h"
#endif

unsigned long previousMillis1000Cycle = 0;
unsigned long interval1000Cycle = 1000;
unsigned long previousMillis10000Cycle = 0;
unsigned long interval10000Cycle = 10000;

void setup(){
  Serial.begin(115200);
  Serial.println("");
  Log.printf("Setting things up ...\r\n");

  #ifdef useWIFI
  wifi_setup();
  wifi_enable();
  #endif
  #if defined(useOTAUpdate)
  OTA_setup("ESP32fancontroller");
  // Do not start OTA. Save heap space and start it via MQTT only when needed.
  // ArduinoOTA.begin();
  #endif
  #if defined(useTelnetStream)
  TelnetStream.begin();
  #endif
  #ifdef useTFT
  initTFT();
  #endif
  #ifdef useTouch
  initTFTtouch();
  #endif
  initPWMfan();
  initTacho();
  #ifdef useTemperatureSensorBME280
  initBME280();
  #endif
  #ifdef useAutomaticTemperatureControl
  initTemperatureController();
  #endif

  Log.printf("Settings done. Have fun.\r\n");
}

void loop(){
  // functions that shall be called as often as possible
  // these functions should take care on their own that they don't nee too much time
  updateTacho();
  #ifdef useTouch
  processUserInput();
  #endif
  #if defined(useOTAUpdate) && !defined(useOTA_RTOS)
  // If you do not use FreeRTOS, you have to regulary call the handle method
  ArduinoOTA.handle();
  #endif
  // mqtt_loop() is doing mqtt keepAlive, processes incoming messages and hence triggers callback
  #ifdef useMQTT
  mqtt_loop();
  #endif

  unsigned long currentMillis = millis();

  // functions that shall be called every 1000 ms
  if ((currentMillis - previousMillis1000Cycle) >= interval1000Cycle) {
    previousMillis1000Cycle = currentMillis;

    #ifdef useTemperatureSensorBME280
    updateBME280();
    #endif
    #ifdef useAutomaticTemperatureControl
    setFanPWMbasedOnTemperature();
    #endif
    #ifdef useTFT
    draw_screen();
    #endif
  }

  // functions that shall be called every 10000 ms
  if ((currentMillis - previousMillis10000Cycle) >= interval10000Cycle) {
    previousMillis10000Cycle = currentMillis;

    #ifdef useMQTT
    mqtt_publish_tele();
    #endif
    doLog();
  }
}
#include <Arduino.h>
#include "sensorBME280.h"
#include "config.h"
#include "fanPWM.h"
#include "fanTacho.h"
#include "log.h"
#include "mqtt.h"
#include "temperatureController.h"
#include "tft.h"
#include "tftTouch.h"
#include "wifiCommunication.h"
#include "DHT.h"   // Librairie des capteurs DHT  https://github.com/adafruit/DHT-sensor-library
#include "sensorDHT.h"   

unsigned long millisecondsSinceStart = 0;
unsigned long millisecondsLast1000Cycle = 0;
unsigned long millisecondsLast10000Cycle = 0;

void setup(){
  Serial.begin(115200);
  Serial.println("");
  log_printf(MY_LOG_FORMAT("Setting things up ..."));
  #ifdef useWIFI
  setup_wifi();
  #endif
  #ifdef useMQTT
  setup_mqtt();
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
  #ifdef useTemperatureSensorDHT
  initDHT();
   #endif
  #ifdef useAutomaticTemperatureControl
  initTemperatureController();
  #endif

  // set to current millis()
  millisecondsLast1000Cycle = millis();
  millisecondsLast10000Cycle = millis();

}

void loop(){
  // functions that shall be called as often as possible
  // these functions should take care on their own that they don't nee too much time
  updateTacho();
  #ifdef useTouch
  processUserInput();
  #endif
  // mqtt_loop() is doing mqtt keepAlive, processes incoming messages and hence triggers callback
  #ifdef useMQTT
  mqtt_loop();
  #endif

  // functions that shall be called every 1000 ms
  millisecondsSinceStart = millis();
  if ((unsigned long)(millisecondsSinceStart - millisecondsLast1000Cycle) >= 1000) {
    #ifdef useTemperatureSensorBME280
    updateBME280();
    #endif
    #ifdef useTemperatureSensorDHT
    updateDHT();
    #endif
    #ifdef useAutomaticTemperatureControl
    setFanPWMbasedOnTemperature();
    #endif
    #ifdef useTFT
    draw_screen();
    #endif

    millisecondsLast1000Cycle = millisecondsSinceStart;
  }

  // functions that shall be called every 10000 ms
  millisecondsSinceStart = millis();
  if ((unsigned long)(millisecondsSinceStart - millisecondsLast10000Cycle) >= 10000) {
    #ifdef useMQTT
    mqtt_publish_tele();
    #endif
    doLog();

    millisecondsLast10000Cycle = millisecondsSinceStart;
  }
}
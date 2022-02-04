#include <Arduino.h>
#include <ArduinoOTA.h>
#if defined(ESP32)
  #include <WiFi.h>
#endif
#if defined(ESP8266)
  #include <ESP8266WiFi.h> 
#endif
#include <WiFiClient.h>
#include <PubSubClient.h>

#include "config.h"
#include "log.h"
#include "wifiCommunication.h"
#include "mqtt.h"
#include "fanPWM.h"
#include "fanTacho.h"
#include "sensorBME280.h"
#include "temperatureController.h"
#include "tft.h"

#ifdef useMQTT
// https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
// https://github.com/knolleary/pubsubclient
// https://gist.github.com/igrr/7f7e7973366fc01d6393

unsigned long reconnectInterval = 5000;
// in order to do reconnect immediately ...
unsigned long lastReconnectAttempt = millis() - reconnectInterval - 1;

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient wifiClient;

PubSubClient mqttClient(mqtt_server, mqtt_server_port, callback, wifiClient);

bool checkMQTTconnection();

void mqtt_loop(){
  if (!mqttClient.connected()) {
    unsigned long currentMillis = millis();
    if ((currentMillis - lastReconnectAttempt) > reconnectInterval) {
      lastReconnectAttempt = currentMillis;
      // Attempt to reconnect
      checkMQTTconnection();
    }
  }  

  if (mqttClient.connected()) {
    mqttClient.loop();
  }
}

bool checkMQTTconnection() {
  if (wifiIsDisabled) return false;

  if (WiFi.isConnected()) {
    if (mqttClient.connected()) {
      return true;
    } else {
      // try to connect to mqtt server
      if (mqttClient.connect((char*) mqtt_clientName, (char*) mqtt_user, (char*) mqtt_pass)) {
        Log.printf("  Successfully connected to MQTT broker\r\n");
    
        // subscribes to messages with given topic.
        // Callback function will be called 1. in client.loop() 2. when sending a message
        mqttClient.subscribe(mqttCmndTargetTemp);
        mqttClient.subscribe(mqttCmndActualTemp);
        mqttClient.subscribe(mqttCmndFanPWM);
        #if defined(useOTAUpdate)
        mqttClient.subscribe(mqttCmndOTA);
        #endif
      } else {
        Log.printf("  MQTT connection failed (but WiFi is available). Will try later ...\r\n");
      }
      return mqttClient.connected();
    }
  } else {
    Log.printf("  No connection to MQTT server, because WiFi ist not connected.\r\n");
    return false;
  }  
}

bool publishMQTTMessage( const char *topic, const char *payload){
  if (wifiIsDisabled) return false;

  if (checkMQTTconnection()) {
//  Log.printf("Sending mqtt payload to topic \"%s\": %s\r\n", topic, payload);
      
    if (mqttClient.publish(topic, payload)) {
      // Log.printf("Publish ok\r\n");
      return true;
    }
    else {
      Log.printf("Publish failed\r\n");
    }
  } else {
    Log.printf("  Cannot publish mqtt message, because checkMQTTconnection failed (WiFi or mqtt is not connected)\r\n");
  }
  return false;
}

bool mqtt_publish_stat_targetTemp() {
  return publishMQTTMessage(mqttStatTargetTemp, ((String)getTargetTemperature()).c_str());
};
bool mqtt_publish_stat_actualTemp() {
  return publishMQTTMessage(mqttStatActualTemp, ((String)getActualTemperature()).c_str());
};
bool mqtt_publish_stat_fanPWM() {
  return publishMQTTMessage(mqttStatFanPWM,     ((String)getPWMvalue()).c_str());
};

bool mqtt_publish_tele() {
  bool error = false;
  // maximum message length 128 Byte
  String payload = "";
  // BME280
  #ifdef useTemperatureSensorBME280
  payload += "{\"ActTemp\":";
  payload += lastTempSensorValues[0];
  payload += ",\"pres\":";
  payload += lastTempSensorValues[1];
  payload += ",\"alt\":";
  payload += lastTempSensorValues[2];
  payload += ",\"hum\":";
  payload += lastTempSensorValues[3];
  payload += ",\"TargTemp\":";
  payload += getTargetTemperature();
  payload += "}";
  if (!publishMQTTMessage(mqttTeleState1, payload.c_str())) error = true;
  #endif

  // Fan
  payload = "";
  payload += "{\"rpm\":";
  payload += last_rpm;
  payload += ",\"pwm\":";
  payload += getPWMvalue();
  payload += "}";
  if (!publishMQTTMessage(mqttTeleState2, payload.c_str())) error = true;

  // WiFi
  payload = "";
  payload += "{\"wifiRSSI\":";
  payload += WiFi.RSSI();
  payload += ",\"wifiChan\":";
  payload += WiFi.channel();
  payload += ",\"wifiSSID\":";
  payload += WiFi.SSID();
  payload += ",\"wifiBSSID\":";
  payload += WiFi.BSSIDstr();
  payload += ",\"IP\":";
  payload += WiFi.localIP().toString();
  payload += "}";
  if (!publishMQTTMessage(mqttTeleState3, payload.c_str())) error = true;

  // ESP32 stats
  payload = "";
  payload += "{\"up\":";
  payload += String(millis());
  payload += ",\"heapSize\":";
  payload += String(ESP.getHeapSize());
  payload += ",\"heapFree\":";
  payload += String(ESP.getFreeHeap());
  payload += ",\"heapMin\":";
  payload += String(ESP.getMinFreeHeap());
  payload += ",\"heapMax\":";
  payload += String(ESP.getMaxAllocHeap());
  payload += "}";
  if (!publishMQTTMessage(mqttTeleState4, payload.c_str())) error = true;

  return error;
}

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  std::string strPayload(reinterpret_cast<const char *>(payload), length);

  Log.printf("MQTT message arrived [%s] %s\r\n", topic, strPayload.c_str());

  String topicReceived(topic);

  String topicCmndTargetTemp(mqttCmndTargetTemp);
  String topicCmndActualTemp(mqttCmndActualTemp);
  String topicCmndFanPWM(mqttCmndFanPWM);
  #if defined(useOTAUpdate)
  String topicCmndOTA(mqttCmndOTA);
  #endif
  if (topicReceived == topicCmndTargetTemp) {
    #ifdef useAutomaticTemperatureControl
    Log.printf("Setting targetTemp via mqtt\r\n");
    float num_float = ::atof(strPayload.c_str());
    Log.printf("new targetTemp: %.2f\r\n", num_float);
    updatePWM_MQTT_Screen_withNewTargetTemperature(num_float, true);
    #else
    Log.printf("\"#define useAutomaticTemperatureControl\" is NOT used in config.h Cannot set target temperature. Please set fan pwm.\r\n");
    updatePWM_MQTT_Screen_withNewTargetTemperature(getTargetTemperature(), true);
    #endif
  } else if (topicReceived == topicCmndActualTemp) {
    #if defined(useAutomaticTemperatureControl) && defined(setActualTemperatureViaMQTT)
    Log.printf("Setting actualTemp via mqtt\r\n");
    float num_float = ::atoi(strPayload.c_str());
    Log.printf("new actualTemp: %.2f\r\n", num_float);
    updatePWM_MQTT_Screen_withNewActualTemperature(num_float, true);
    #else
    Log.printf("\"#define setActualTemperatureViaMQTT\" is NOT used in config.h  Cannot set actual temperature. Please use BME280.\r\n");
    updatePWM_MQTT_Screen_withNewActualTemperature(getActualTemperature(), true);
    #endif
  } else if (topicReceived == topicCmndFanPWM) {
    #ifndef useAutomaticTemperatureControl
    Log.printf("Setting fan pwm via mqtt\r\n");
    int num_int = ::atoi(strPayload.c_str());
    Log.printf("new fan pwm: %d\r\n", num_int);
    updateMQTT_Screen_withNewPWMvalue(num_int, true);
    #else
    Log.printf("\"#define useAutomaticTemperatureControl\" is used in config.h  Cannot set fan pwm. Please set target temperature.\r\n");
    updateMQTT_Screen_withNewPWMvalue(getPWMvalue(), true);
    #endif
#if defined(useOTAUpdate)
  } else if (topicReceived == topicCmndOTA) {
    if (strPayload == "ON") {
      Log.printf("MQTT command TURN ON OTA received\r\n");
      ArduinoOTA.begin();
    } else if (strPayload == "OFF") {
      Log.printf("MQTT command TURN OFF OTA received\r\n");
      ArduinoOTA.end();
    } else {
      Log.printf("Payload %s not supported\r\n", strPayload.c_str());
    }
#endif
  }
}
#endif

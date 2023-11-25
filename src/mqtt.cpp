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
#ifdef useHomeassistantMQTTDiscovery
unsigned long timerStartForHAdiscovery = 1;
#endif

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient wifiClient;

PubSubClient mqttClient(MQTT_SERVER, MQTT_SERVER_PORT, callback, wifiClient);

bool checkMQTTconnection();

void mqtt_setup() {
  #ifdef useHomeassistantMQTTDiscovery
  // Set buffer size to allow hass discovery payload
  mqttClient.setBufferSize(1280);
  #endif
}

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
      #if !defined(useHomeassistantMQTTDiscovery)
      if (mqttClient.connect(MQTT_CLIENTNAME, MQTT_USER, MQTT_PASS)) {
      #else
      // In case of Home Assistant, connect with last will to the broker to set the device offline when the esp32 fan controller is swtiched off
      if (mqttClient.connect(MQTT_CLIENTNAME, MQTT_USER, MQTT_PASS,
                             HASSFANSTATUSTOPIC, 0, 1, HASSSTATUSOFFLINEPAYLOAD)) {
      #endif
        Log.printf("  Successfully connected to MQTT broker\r\n");
    
        // subscribes to messages with given topic.
        // Callback function will be called 1. in client.loop() 2. when sending a message
        mqttClient.subscribe(MQTTCMNDTARGETTEMP);
        mqttClient.subscribe(MQTTCMNDACTUALTEMP);
        mqttClient.subscribe(MQTTCMNDFANPWM);
        mqttClient.subscribe(MQTTCMNDFANMODE);
        #if defined(useOTAUpdate)
        mqttClient.subscribe(MQTTCMNDOTA);
        #endif
        #if defined(useHomeassistantMQTTDiscovery)
        mqttClient.subscribe(HASSSTATUSTOPIC);
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

bool publishMQTTMessage(const char *topic, const char *payload, boolean retained){
  if (wifiIsDisabled) return false;

  if (checkMQTTconnection()) {
//  Log.printf("Sending mqtt payload to topic \"%s\": %s\r\n", topic, payload);
      
    if (mqttClient.publish(topic, payload, retained)) {
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

bool publishMQTTMessage(const char *topic, const char *payload){
  return publishMQTTMessage(topic, payload, false);
}

bool mqtt_publish_stat_targetTemp() {
  return publishMQTTMessage(MQTTSTATTARGETTEMP, ((String)getTargetTemperature()).c_str());
};
bool mqtt_publish_stat_actualTemp() {
  return publishMQTTMessage(MQTTSTATACTUALTEMP, ((String)getActualTemperature()).c_str());
};
bool mqtt_publish_stat_fanPWM() {
  return publishMQTTMessage(MQTTSTATFANPWM,     ((String)getPWMvalue()).c_str());
};
bool mqtt_publish_stat_mode() {
  return publishMQTTMessage(MQTTSTATFANMODE,    getModeIsOff() ? MQTTFANMODEOFFPAYLOAD : MQTTFANMODEFANONLYPAYLOAD);
};

#ifdef useHomeassistantMQTTDiscovery
bool mqtt_publish_hass_discovery() {
  Log.printf("Will send HA discovery now.\r\n");
  bool error = false;
  error =          !publishMQTTMessage(HASSCLIMATEDISCOVERYTOPIC,           HASSCLIMATEDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSHUMIDITYSENSORDISCOVERYTOPIC,    HASSHUMIDITYSENSORDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSTEMPERATURESENSORDISCOVERYTOPIC, HASSTEMPERATURESENSORDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSPRESSURESENSORDISCOVERYTOPIC,    HASSPRESSURESENSORDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSALTITUDESENSORDISCOVERYTOPIC,    HASSALTITUDESENSORDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSPWMSENSORDISCOVERYTOPIC,         HASSPWMSENSORDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSRPMSENSORDISCOVERYTOPIC,         HASSRPMSENSORDISCOVERYPAYLOAD);

  if (!error) {delay(1000);}
  // publish that we are online. Remark: offline is sent via last will retained message
  error = error || !publishMQTTMessage(HASSFANSTATUSTOPIC, "", true);
  error = error || !publishMQTTMessage(HASSFANSTATUSTOPIC, HASSSTATUSONLINEPAYLOAD);

  if (!error) {delay(1000);}
  // MODE????

  // that's not really part of the discovery message, but this enables the climate slider in HA and immediately provides all values
  error = error || !mqtt_publish_stat_targetTemp();
  error = error || !mqtt_publish_stat_actualTemp();
  error = error || !mqtt_publish_stat_fanPWM();
  error = error || !mqtt_publish_stat_mode();
  error = error || !mqtt_publish_tele();
  if (!error) {
    // will not resend discovery as long as timerStartForHAdiscovery == 0
    Log.printf("Will set timer to 0 now, this means I will not send discovery again.\r\n");
    timerStartForHAdiscovery = 0;
  } else {
    Log.printf("Some error occured while sending discovery. Will try again.\r\n");
  }
  return !error;
}
#endif

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
  error =          !publishMQTTMessage(MQTTTELESTATE1, payload.c_str());
  #endif

  // Fan
  payload = "";
  payload += "{\"rpm\":";
  payload += last_rpm;
  payload += ",\"pwm\":";
  payload += getPWMvalue();
  payload += "}";
  error = error || !publishMQTTMessage(MQTTTELESTATE2, payload.c_str());

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
  #if defined(WIFI_KNOWN_APS)
  payload += ",\"wifiAP\":";
  payload += accessPointName;
  #endif
  payload += ",\"IP\":";
  payload += WiFi.localIP().toString();
  payload += "}";
  error = error || !publishMQTTMessage(MQTTTELESTATE3, payload.c_str());

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
  error = error || !publishMQTTMessage(MQTTTELESTATE4, payload.c_str());

  return !error;
}

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  std::string strPayload(reinterpret_cast<const char *>(payload), length);

  Log.printf("MQTT message arrived [%s] %s\r\n", topic, strPayload.c_str());

  String topicReceived(topic);

  String topicCmndTargetTemp(MQTTCMNDTARGETTEMP);
  String topicCmndActualTemp(MQTTCMNDACTUALTEMP);
  String topicCmndFanPWM(MQTTCMNDFANPWM);
  String topicCmndFanMode(MQTTCMNDFANMODE);
  #if defined(useOTAUpdate)
  String topicCmndOTA(MQTTCMNDOTA);
  #endif
  #if defined(useHomeassistantMQTTDiscovery)
  String topicHaStatus(HASSSTATUSTOPIC);
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
  } else if (topicReceived == topicCmndFanMode) {
    Log.printf("Setting HVAC mode from HA received via mqtt\r\n");
    if (strPayload == MQTTFANMODEFANONLYPAYLOAD) {
      Log.printf("  Will turn fan into \"fan_only\" mode\r\n");
      updateMQTT_Screen_withNewMode(false, true);
    } else if (strPayload == MQTTFANMODEOFFPAYLOAD) {
      Log.printf("  Will switch fan off\r\n");
      updateMQTT_Screen_withNewMode(true, true);
    } else {
      Log.printf("Payload %s not supported\r\n", strPayload.c_str());
    }
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
#if defined(useHomeassistantMQTTDiscovery)
  } else if (topicReceived == topicHaStatus) {
    if (strPayload == HASSSTATUSONLINEPAYLOAD) {
      Log.printf("HA status online received. This means HA has restarted. Will send discovery again in some seconds as defined in config.h\r\n");
      // set timer so that discovery will be resent after some seconds (as defined in config.h)
      timerStartForHAdiscovery = millis();
      // Very unlikely. Can only happen if millis() overflowed max unsigned long every approx. 50 days
      if (timerStartForHAdiscovery == 0) {timerStartForHAdiscovery = 1;}
    } else if (strPayload == HASSSTATUSOFFLINEPAYLOAD) {
      Log.printf("HA status offline received. Nice to know. Currently we don't react to this.\r\n");
    } else {
      Log.printf("Payload %s not supported\r\n", strPayload.c_str());
    }
#endif
  }
}
#endif

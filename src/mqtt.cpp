#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "config.h"
#include "fanPWM.h"
#include "fanPWM.h"
#include "fanTacho.h"
#include "log.h"
#include "sensorBME280.h"
#include "temperatureController.h"
#include "tft.h"
#include "wifiCommunication.h"

#ifdef useMQTT
// https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
// https://github.com/knolleary/pubsubclient
// https://gist.github.com/igrr/7f7e7973366fc01d6393
long lastReconnectAttempt = 0;
void callback(char* topic, byte* payload, unsigned int length);
bool checkConnection();
bool reconnect();

WiFiClient wifiClient;

PubSubClient client(mqtt_server, mqtt_server_port, callback, wifiClient);
#endif

void setup_mqtt(){
  #ifdef useMQTT
  // in order to do reconnect immediately ...
  lastReconnectAttempt = millis() - 5001;
  if (checkConnection()){
    log_printf(MY_LOG_FORMAT("  MQTT sucessfully initialized."));
  } else {
    log_printf(MY_LOG_FORMAT("  MQTT connection failed."));
  };
  #else
  log_printf(MY_LOG_FORMAT("    MQTT is disabled in config.h"));
  #endif
}

void mqtt_loop(){
  #ifdef useMQTT
  checkConnection(); // checkConnection includes client.loop()
  #endif
}

#ifdef useMQTT
bool checkConnection(){
  if (!checkWiFiAndReconnectTwice()){
    log_printf(MY_LOG_FORMAT("  Cannot connect to MQTT server. No WiFi connection."));
    return false;
  }
  if (!client.connected()) {
    long now = millis();
    if ((unsigned long)(now - lastReconnectAttempt) > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    client.loop();
  }
  return client.connected();
}

bool reconnect() {
  if (client.connect((char*) mqtt_clientName, (char*) mqtt_user, (char*) mqtt_pass)) {
    log_printf(MY_LOG_FORMAT("  Connected to MQTT broker"));
    
    // subscribes to messages with given topic.
    // Callback function will be called 1. in client.loop() 2. when sending a message
    client.subscribe(mqttCmndTargetTemp);
    client.subscribe(mqttCmndActualTemp);
    client.subscribe(mqttCmndFanPWM);

  } else {
    log_printf(MY_LOG_FORMAT("  MQTT connection failed. Will try later ..."));
  }
  return client.connected();
}

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  std::string strPayload(reinterpret_cast<const char *>(payload), length);

  log_printf(MY_LOG_FORMAT("MQTT message arrived [%s] %s"), topic, strPayload.c_str());

  String topicReceived(topic);
  String topicCmndTargetTemp(mqttCmndTargetTemp);
  String topicCmndActualTemp(mqttCmndActualTemp);
  String topicCmndFanPWM(mqttCmndFanPWM);
  if (topicReceived == topicCmndTargetTemp) {
    #ifdef useAutomaticTemperatureControl
    log_printf(MY_LOG_FORMAT("Setting targetTemp via mqtt"));
    float num_float = ::atof(strPayload.c_str());
    log_printf(MY_LOG_FORMAT("new targetTemp: %.2f"), num_float);
    updatePWM_MQTT_Screen_withNewTargetTemperature(num_float, true);
    #else
    log_printf(MY_LOG_FORMAT("\"#define useAutomaticTemperatureControl\" is NOT used in config.h Cannot set target temperature. Please set fan pwm."));
    updatePWM_MQTT_Screen_withNewTargetTemperature(getTargetTemperature(), true);
    #endif
  } else if (topicReceived == topicCmndActualTemp) {
    #if defined(useAutomaticTemperatureControl) && defined(setActualTemperatureViaMQTT)
    log_printf(MY_LOG_FORMAT("Setting actualTemp via mqtt"));
    float num_float = ::atoi(strPayload.c_str());
    log_printf(MY_LOG_FORMAT("new actualTemp: %.2f"), num_float);
    updatePWM_MQTT_Screen_withNewActualTemperature(num_float, true);
    #else
    log_printf(MY_LOG_FORMAT("\"#define setActualTemperatureViaMQTT\" is NOT used in config.h  Cannot set actual temperature. Please use BME280."));
    updatePWM_MQTT_Screen_withNewActualTemperature(getActualTemperature(), true);
    #endif
  } else if (topicReceived == topicCmndFanPWM) {
    #ifndef useAutomaticTemperatureControl
    log_printf(MY_LOG_FORMAT("Setting fan pwm via mqtt"));
    int num_int = ::atoi(strPayload.c_str());
    log_printf(MY_LOG_FORMAT("new fan pwm: %d"), num_int);
    updateMQTT_Screen_withNewPWMvalue(num_int, true);
    #else
    log_printf(MY_LOG_FORMAT("\"#define useAutomaticTemperatureControl\" is used in config.h  Cannot set fan pwm. Please set target temperature."));
    updateMQTT_Screen_withNewPWMvalue(getPWMvalue(), true);
    #endif
  }
}

void publishMQTTMessage( const char *topic, const char *payload){
  // don't do loop here, messages to send get in wrong order
//if (checkConnection()) { // checkConnection includes client.loop()
    if (client.connected()){
      log_printf(MY_LOG_FORMAT("Sending mqtt payload to topic \"%s\": %s"), topic, payload);
      
      if (client.publish(topic, payload)) {
        // log_printf(MY_LOG_FORMAT("Publish ok"));
      }
      else {
        log_printf(MY_LOG_FORMAT("Publish failed"));
      }
    }
//};  
}

void mqtt_publish_stat_targetTemp() {
  publishMQTTMessage(mqttStatTargetTemp, ((String)getTargetTemperature()).c_str());
};
void mqtt_publish_stat_actualTemp() {
  publishMQTTMessage(mqttStatActualTemp, ((String)getActualTemperature()).c_str());
};
void mqtt_publish_stat_fanPWM() {
  publishMQTTMessage(mqttStatFanPWM,     ((String)getPWMvalue()).c_str());
};
#endif

void mqtt_publish_tele() {
  #ifdef useMQTT
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
  publishMQTTMessage(mqttTeleState1, payload.c_str());
  #endif

  // Fan
  payload = "";
  payload += "{\"rpm\":";
  payload += last_rpm;
  payload += ",\"pwm\":";
  payload += getPWMvalue();
  payload += "}";
  publishMQTTMessage(mqttTeleState2, payload.c_str());

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
  payload += "}";
  publishMQTTMessage(mqttTeleState3, payload.c_str());
  #endif
}

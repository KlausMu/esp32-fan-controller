#include <Arduino.h>
#include <esp32-hal-log.h>
#include <math.h>
#include "config.h"
#include "fanPWM.h"
#include "log.h"
#include "mqtt.h"
#include "sensorBME280.h"
#include "tft.h"
#include "sensorDHT.h"
#include "DHT.h"

float targetTemperature;
float actualTemperature;
void setActualTemperatureAndPublishMQTT(float aActualTemperature) {
  if (actualTemperature != aActualTemperature) {
    actualTemperature = aActualTemperature;
    #ifdef useMQTT
    mqtt_publish_stat_actualTemp();
    #endif
  }
}

void updatePWM_MQTT_Screen_withNewTargetTemperature(float aTargetTemperature, bool force);
void updatePWM_MQTT_Screen_withNewActualTemperature(float aActualTemperature, bool force);

void initTemperatureController(void) {
  #ifdef useAutomaticTemperatureControl
  targetTemperature = initialTargetTemperature;
  updatePWM_MQTT_Screen_withNewTargetTemperature(targetTemperature, true);
  #ifdef setActualTemperatureViaMQTT
  setActualTemperatureAndPublishMQTT(NAN);
  updatePWM_MQTT_Screen_withNewActualTemperature(actualTemperature, true);
  #endif

  #else
  log_printf(MY_LOG_FORMAT("    Temperature control is disabled in config.h"));
  #endif
}

float getTargetTemperature(void) {
  return targetTemperature;
}
float getActualTemperature(void) {
  return actualTemperature;
}

void setFanPWMbasedOnTemperature(void) {
  #ifdef useAutomaticTemperatureControl
  float difftemp = getActualTemperature() - targetTemperature;
  int newPWMvalue = 255;

  if ((getActualTemperature() == NAN) || (getActualTemperature() <= 0.0)){
    log_printf(MY_LOG_FORMAT("WARNING: no temperature value available. Cannot do temperature control. Will set PWM fan to 255."));
    newPWMvalue = 255;
  } else if (difftemp <= 0.0) {
    // Temperature is below target temperature. Run fan at minimum speed.
    newPWMvalue = pwmMinimumValue; 
  } else if (difftemp <= 0.5) {
    newPWMvalue = 140;
  } else if (difftemp <= 1.0) {
    newPWMvalue = 160;
  } else if (difftemp <= 1.5) {
    newPWMvalue = 180;
  } else if (difftemp <= 2.0) {
    newPWMvalue = 200;
  } else if (difftemp <= 2.5) {
    newPWMvalue = 220;
  } else if (difftemp <= 3.0) {
    newPWMvalue = 240;
  } else {
    // Temperature much too high. Run fan at full speed.
    newPWMvalue = 255;
  }
  

  updateMQTT_Screen_withNewPWMvalue(newPWMvalue, false);
  #endif
}

void updatePWM_MQTT_Screen_withNewTargetTemperature(float aTargetTemperature, bool force) {
  if ((targetTemperature != aTargetTemperature) || force) {
    targetTemperature = aTargetTemperature;
    setFanPWMbasedOnTemperature();
    #ifdef useMQTT
    mqtt_publish_stat_targetTemp();
    #endif
    draw_screen();
  }
}

void updatePWM_MQTT_Screen_withNewActualTemperature(float aActualTemperature, bool force) {
  if ((actualTemperature != aActualTemperature) || force) {
    actualTemperature = aActualTemperature;
    setFanPWMbasedOnTemperature();
    #ifdef useMQTT
    mqtt_publish_stat_actualTemp();
    #endif
    draw_screen();
  }
}
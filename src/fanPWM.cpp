#include <Arduino.h>
#include <esp32-hal.h>
#include <esp32-hal-ledc.h>
#include "config.h"
#include "log.h"
#include "mqtt.h"
#include "tft.h"

int pwmValue = 0;
void updateMQTT_Screen_withNewPWMvalue(int aPWMvalue, bool force);

// https://randomnerdtutorials.com/esp32-pwm-arduino-ide/
void initPWMfan(void){
  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(pwmPin, pwmChannel);

  pwmValue = initialPwmValue;
  updateMQTT_Screen_withNewPWMvalue(pwmValue, true);

  Log.printf("  Fan PWM sucessfully initialized.\r\n");
}

void updateFanSpeed(void){
  ledcWrite(pwmChannel, pwmValue);
}

void updateMQTT_Screen_withNewPWMvalue(int aPWMvalue, bool force) {
  if ((pwmValue != aPWMvalue) || force) {
    pwmValue = aPWMvalue;
    if (pwmValue < 0) {pwmValue = 0;};
    if (pwmValue > 255) {pwmValue = 255;};
    updateFanSpeed();
    #ifdef useMQTT
    mqtt_publish_stat_fanPWM();
    #endif
    draw_screen();
  }
}

#ifndef useAutomaticTemperatureControl
void incFanSpeed(void){
  int newPWMValue = min(pwmValue+pwmStep, 255);
  updateMQTT_Screen_withNewPWMvalue(newPWMValue, false);
}
void decFanSpeed(void){
  int newPWMValue = max(pwmValue-pwmStep, 0);
  updateMQTT_Screen_withNewPWMvalue(newPWMValue, false);
}
#endif

int getPWMvalue(){
  return pwmValue;
}

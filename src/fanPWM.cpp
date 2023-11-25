#include <Arduino.h>
#include <esp32-hal.h>
#include <esp32-hal-ledc.h>
#include "config.h"
#include "log.h"
#include "mqtt.h"
#include "tft.h"

int pwmValue = 0;
bool modeIsOff = false;
void updateMQTT_Screen_withNewPWMvalue(int aPWMvalue, bool force);
void updateMQTT_Screen_withNewMode(bool aModeIsOff, bool force);

// https://randomnerdtutorials.com/esp32-pwm-arduino-ide/
void initPWMfan(void){
  // configure LED PWM functionalitites
  ledcSetup(PWMCHANNEL, PWMFREQ, PWMRESOLUTION);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PWMPIN, PWMCHANNEL);

  pwmValue = INITIALPWMVALUE;
  updateMQTT_Screen_withNewPWMvalue(pwmValue, true);
  updateMQTT_Screen_withNewMode(false, true);

  Log.printf("  Fan PWM sucessfully initialized.\r\n");
}

void updateFanSpeed(void){
  ledcWrite(PWMCHANNEL, pwmValue);
}

void updateMQTT_Screen_withNewPWMvalue(int aPWMvalue, bool force) {
  // note: it is not guaranteed that fan stops if pwm is set to 0
  if (modeIsOff) {aPWMvalue = 0;}
  if ((pwmValue != aPWMvalue) || force) {
    pwmValue = aPWMvalue;
    if (pwmValue < 0) {pwmValue = 0;};
    if (pwmValue > 255) {pwmValue = 255;};
    updateFanSpeed();
    #ifdef useMQTT
    mqtt_publish_stat_fanPWM();
    mqtt_publish_tele();
    #endif
    draw_screen();
  }
}

void updateMQTT_Screen_withNewMode(bool aModeIsOff, bool force) {
  if ((modeIsOff != aModeIsOff) || force) {
    modeIsOff = aModeIsOff;
    #ifdef useMQTT
    mqtt_publish_stat_mode();
    #endif
    switchOff_screen(modeIsOff);
  }
  if (modeIsOff) {
    updateMQTT_Screen_withNewPWMvalue(0, true);
  } else {
    updateMQTT_Screen_withNewPWMvalue(INITIALPWMVALUE, true);
  }
}

#ifndef useAutomaticTemperatureControl
void incFanSpeed(void){
  int newPWMValue = min(pwmValue+PWMSTEP, 255);
  updateMQTT_Screen_withNewPWMvalue(newPWMValue, false);
}
void decFanSpeed(void){
  int newPWMValue = max(pwmValue-PWMSTEP, 0);
  updateMQTT_Screen_withNewPWMvalue(newPWMValue, false);
}
#endif

int getPWMvalue(){
  return pwmValue;
}

bool getModeIsOff(void) {
  return modeIsOff;
}

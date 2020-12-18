#include <Arduino.h>
#include <esp32-hal-log.h>
#include "config.h"
#include "log.h"
#include "sensorBME280.h"
#include "fanPWM.h"
#include "fanTacho.h"
#include "temperatureController.h"

void doLog(void){
  #ifdef useTemperatureSensorBME280
  log_printf(MY_LOG_FORMAT("actual temperature = %.2f *C, pressure = %.2f hPa, approx. altitude = %.2f m, humidity = %.2f %%"), lastTempSensorValues[0], lastTempSensorValues[1], lastTempSensorValues[2], lastTempSensorValues[3]);
  #endif
  #ifdef useAutomaticTemperatureControl
  log_printf(MY_LOG_FORMAT("target temperature = %.2f *C"), getTargetTemperature());
  #endif
  log_printf(MY_LOG_FORMAT("fan rpm = %d, fan pwm = %d"), last_rpm, getPWMvalue());
}
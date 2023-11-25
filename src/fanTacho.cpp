#include <Arduino.h>
#include <esp32-hal.h>
#include <pins_arduino.h>
#include "config.h"
#include "log.h"

static volatile int counter_rpm = 0;
int last_rpm = 0;
unsigned long millisecondsLastTachoMeasurement = 0;

// Interrupt counting every rotation of the fan
// https://desire.giesecke.tk/index.php/2018/01/30/change-global-variables-from-isr/
void IRAM_ATTR rpm_fan() {
  counter_rpm++;
}

void initTacho(void) {
  pinMode(TACHOPIN, INPUT);
  digitalWrite(TACHOPIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(TACHOPIN), rpm_fan, FALLING);
  Log.printf("  Fan tacho detection sucessfully initialized.\r\n");
}

void updateTacho(void) {
  // start of tacho measurement
  if ((unsigned long)(millis() - millisecondsLastTachoMeasurement) >= TACHOUPDATECYCLE)
  { 
    // detach interrupt while calculating rpm
    detachInterrupt(digitalPinToInterrupt(TACHOPIN)); 
    // calculate rpm
    last_rpm = counter_rpm * ((float)60 / (float)NUMBEROFINTERRUPSINONESINGLEROTATION) * ((float)1000 / (float)TACHOUPDATECYCLE);
    // Log.printf("fan rpm = %d\r\n", last_rpm);

    // reset counter
    counter_rpm = 0; 
    // store milliseconds when tacho was measured the last time
    millisecondsLastTachoMeasurement = millis();

    // attach interrupt again
    attachInterrupt(digitalPinToInterrupt(TACHOPIN), rpm_fan, FALLING);
  }
}
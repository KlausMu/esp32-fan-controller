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
  pinMode(tachoPin, INPUT);
  digitalWrite(tachoPin, HIGH);
  attachInterrupt(digitalPinToInterrupt(tachoPin), rpm_fan, FALLING);
  log_printf(MY_LOG_FORMAT("  Fan tacho detection sucessfully initialized."));
}

void updateTacho(void) {
  // start of tacho measurement
  if ((unsigned long)(millis() - millisecondsLastTachoMeasurement) >= tachoUpdateCycle)
  { 
    // detach interrupt while calculating rpm
    detachInterrupt(digitalPinToInterrupt(tachoPin)); 
    // calculate rpm
    last_rpm = counter_rpm * (60 / numberOfInterrupsInOneSingleRotation);
    // log_printf(MY_LOG_FORMAT("fan rpm = %d"), last_rpm);

    // reset counter
    counter_rpm = 0; 
    // store milliseconds when tacho was measured the last time
    millisecondsLastTachoMeasurement = millis();

    // attach interrupt again
    attachInterrupt(digitalPinToInterrupt(tachoPin), rpm_fan, FALLING);
  }
}
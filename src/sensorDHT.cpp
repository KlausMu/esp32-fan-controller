#include "config.h"
#include "log.h"
#include "temperatureController.h"
#include "DHT.h"   // Librairie des capteurs DHT  https://github.com/adafruit/DHT-sensor-library
#include 	<float.h>
#ifdef useTemperatureSensorDHT

float lastTempSensorValues[4];
struct DHT dht(DHTPIN, DHTTYPE); 
bool status_DHT = 0;
#endif

void initDHT(void){
  #ifdef useTemperatureSensorDHT
  lastTempSensorValues[0] = NAN;
  lastTempSensorValues[1] = NAN;
  lastTempSensorValues[2] = NAN;
  lastTempSensorValues[3] = NAN;
  dht.begin();   // DHT dht(DHTPIN, DHTTYPE); 

    // Lecture du taux d'humidité
   lastTempSensorValues[1] = dht.readHumidity();
   // Lecture de la température en Celcius
   lastTempSensorValues[0] = dht.readTemperature();
   // Pour lire la température en Fahrenheit
   lastTempSensorValues[2] = dht.readTemperature(true);


  status_DHT = isnan(lastTempSensorValues[0]) || isnan(lastTempSensorValues[1]) || isnan(lastTempSensorValues[2]);

  if (status_DHT) {
    log_printf(MY_LOG_FORMAT("  Could not find a valid DHT sensor, check wiring!"));
  } else {
    log_printf(MY_LOG_FORMAT("  DHT sucessfully initialized. ---------------------------"));
    log_printf(MY_LOG_FORMAT("The first actual temperature = %.2f *C, humidity = %.2f %%"), lastTempSensorValues[0], lastTempSensorValues[1]);
  }
  #else
  log_printf(MY_LOG_FORMAT("    DHT is disabled in config.h"));
  #endif
}

void updateDHT(void){
  #ifdef useTemperatureSensorDHT
  if (status_DHT){
    log_printf(MY_LOG_FORMAT("DHT sensor not initialized, trying again ..."));
    initDHT();
    if (!status_DHT){
      log_printf(MY_LOG_FORMAT("success!"));
    } else {
      lastTempSensorValues[0] = NAN;
      #ifndef setActualTemperatureViaMQTT
      setActualTemperatureAndPublishMQTT(lastTempSensorValues[0]);
      #endif
      lastTempSensorValues[1] = NAN;
      lastTempSensorValues[2] = NAN;
      lastTempSensorValues[3] = NAN;
      return;
    }
  }
  lastTempSensorValues[0] = dht.readTemperature();
  #ifndef setActualTemperatureViaMQTT
  setActualTemperatureAndPublishMQTT(lastTempSensorValues[0]);
  #endif
  lastTempSensorValues[1] = dht.readTemperature(true);
  lastTempSensorValues[2] = dht.readHumidity();
  lastTempSensorValues[3] = dht.computeHeatIndex(lastTempSensorValues[1], lastTempSensorValues[2]);
  #endif
}
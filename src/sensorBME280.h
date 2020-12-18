#ifdef useTemperatureSensorBME280
extern float lastTempSensorValues[4];
#endif
void initBME280(void);
void updateBME280(void);
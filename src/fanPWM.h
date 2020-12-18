void initPWMfan(void);
void updateMQTT_Screen_withNewPWMvalue(int aPWMvalue, bool force);
#ifndef useAutomaticTemperatureControl
void incFanSpeed(void);
void decFanSpeed(void);
#endif
int getPWMvalue();

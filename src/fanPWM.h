void initPWMfan(void);
void updateMQTT_Screen_withNewPWMvalue(int aPWMvalue, bool force);
void updateMQTT_Screen_withNewMode(bool aModeIsOff, bool force);
#ifndef useAutomaticTemperatureControl
void incFanSpeed(void);
void decFanSpeed(void);
#endif
int getPWMvalue();
bool getModeIsOff(void);

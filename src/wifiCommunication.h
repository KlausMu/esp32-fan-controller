#ifdef useWIFI
extern bool wifiIsDisabled;
extern String accessPointName;

void wifi_setup(void);
void wifi_enable(void);
void wifi_disable(void);
#endif
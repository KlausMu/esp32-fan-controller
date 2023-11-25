#ifdef useWIFI
extern bool wifiIsDisabled;
#if defined(WIFI_KNOWN_APS)
extern String accessPointName;
#endif

void wifi_setup(void);
void wifi_enable(void);
void wifi_disable(void);
#endif
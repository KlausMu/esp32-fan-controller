#ifdef useMQTT
void mqtt_setup(void);
void mqtt_loop(void);
bool mqtt_publish_tele(void);
bool mqtt_publish_stat_targetTemp();
bool mqtt_publish_stat_actualTemp();
bool mqtt_publish_stat_fanPWM();
bool mqtt_publish_hass_discovery();
#endif
#ifdef useMQTT
void mqtt_loop(void);
bool mqtt_publish_tele(void);
bool mqtt_publish_stat_targetTemp();
bool mqtt_publish_stat_actualTemp();
bool mqtt_publish_stat_fanPWM();
#endif
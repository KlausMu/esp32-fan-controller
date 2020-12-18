void setup_mqtt(void);
void mqtt_loop(void);
void mqtt_publish_tele(void);
#ifdef useMQTT
void mqtt_publish_stat_targetTemp(void);
void mqtt_publish_stat_actualTemp(void);
void mqtt_publish_stat_fanPWM(void);
#endif
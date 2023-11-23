#ifdef useMQTT
void mqtt_setup(void);
void mqtt_loop(void);
bool mqtt_publish_tele(void);
bool mqtt_publish_stat_targetTemp();
bool mqtt_publish_stat_actualTemp();
bool mqtt_publish_stat_fanPWM();
bool mqtt_publish_stat_mode();
#ifdef useHomeassistantMQTTDiscovery
/* Sets the start of the timer until HA discovery is sent.
   It will be waited waitAfterHAisOnlineUntilDiscoveryWillBeSent ms before the discovery is sent.
   0: discovery will not be sent
   >0: discovery will be sent as soon as "waitAfterHAisOnlineUntilDiscoveryWillBeSent" ms are over
*/
extern unsigned long timerStartForHAdiscovery;
bool mqtt_publish_hass_discovery();
#endif
#endif
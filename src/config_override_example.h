/*
Copy this file to "config_override.h"
Any defines from "config.h" in CAPITALS can be overridden in "config_override.h".
All defines having BOTH lowercase and uppercase MUST stay in "config.h". They define the mode the "esp32 fan controller" is running in.
If you add additional overrides here, you have to
  1. first add #undef
  2. add new #define
*/
#undef WIFI_SSID
#undef WIFI_PASSWORD
#undef MQTT_SERVER
#undef MQTT_SERVER_PORT
#undef MQTT_USER
#undef MQTT_PASS
#undef UNIQUE_DEVICE_FRIENDLYNAME
#undef UNIQUE_DEVICE_NAME
#undef MQTTCMNDSHUTDOWNTOPIC
#undef MQTTCMNDSHUTDOWNPAYLOAD
#undef TOUCH_CS
#undef TOUCH_IRQ
#undef LED_ON

#ifdef useWIFI
#define WIFI_SSID            "YourWifiSSID"          // override here
#define WIFI_PASSWORD        "YourWifiPassword"      // override here
#endif

#ifdef useMQTT
#define MQTT_SERVER                "IPAddressOfYourBroker" // override here
#define MQTT_SERVER_PORT           1883                    // override here
#define MQTT_USER                  "myUser or empty"       // override here
#define MQTT_PASS                  "myPassword or empty"   // override here
#define UNIQUE_DEVICE_FRIENDLYNAME "Fan Controller"        // override here
#define UNIQUE_DEVICE_NAME         "esp32_fan_controller"  // override here
#endif

#ifdef useShutdownButton
#define MQTTCMNDSHUTDOWNTOPIC          UNIQUE_DEVICE_NAME "/cmnd/shutdown" // override here
#define MQTTCMNDSHUTDOWNPAYLOAD        "shutdown"                          // override here
#endif

#ifdef useTFT
#define LED_ON           HIGH          // override here
#endif
#ifdef useTouch
#define TOUCH_CS         GPIO_NUM_14   // override here
#define TOUCH_IRQ        GPIO_NUM_27   // override here
#endif

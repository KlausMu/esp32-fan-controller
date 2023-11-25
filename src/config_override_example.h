/*
Copy this file to "config_override.h"
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
#undef SHUTDOWNREQUEST
#undef SHUTDOWNPAYLOAD
#undef SHUTDOWNHEADERNAME1
#undef SHUTDOWNHEADERVALUE1
#undef SHUTDOWNHEADERNAME2
#undef SHUTDOWNHEADERVALUE2
#undef TOUCH_CS
#undef TOUCH_IRQ
#undef LED_ON

#ifdef useWIFI
#define WIFI_SSID            "YourWifiSSID"          // override here
#define WIFI_PASSWORD        "YourWifiPassword"      // override here
#endif

#ifdef useMQTT
#define MQTT_SERVER          "IPAddressOfYourBroker" // override here
#define MQTT_SERVER_PORT     1883                    // override here
#define MQTT_USER            "myUser or empty"       // override here
#define MQTT_PASS            "myPassword or empty"   // override here
#endif

#ifdef showShutdownButton
#define SHUTDOWNREQUEST      "http://<IPAddressOfYourHAserver:8123>/api/services/input_button/press" // override here
#define SHUTDOWNPAYLOAD      "{\"entity_id\": \"input_button.3dprinter_shutdown\"}"                  // override here
#define SHUTDOWNHEADERNAME1  "Authorization"                                                         // override here
#define SHUTDOWNHEADERVALUE1 "Bearer <your bearer token>"                                            // override here
#define SHUTDOWNHEADERNAME2  "Content-Type"                                                          // override here
#define SHUTDOWNHEADERVALUE2 "application/json"                                                      // override here
#endif

#ifdef useTFT
#define LED_ON           HIGH          // override here
#endif
#ifdef useTouch
#define TOUCH_CS         GPIO_NUM_14   // override here
#define TOUCH_IRQ        GPIO_NUM_27   // override here
#endif


/*
  Before changing anything in this file, consider to copy file "config_override_example.h" to file "config_override.h" and to do your changes there.
  Doing so, you will
  - keep your credentials secret
  - most likely never have conflicts with new versions of this file
  Any define in CAPITALS can be moved to "config_override.h".
  All defines having BOTH lowercase and uppercase MUST stay in "config.h". They define the mode the "esp32 fan controller" is running in.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <driver/gpio.h>
#include <esp32-hal-gpio.h>

// --- Begin: choose operation mode -------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
// You have two ways to choose the operation mode:
// 1. either use one of the presets
// 2. or define every low level option manually
// Recommendation is to start with one of the presets.
// In both cases, after choosing the operation mode, go further down in this file to set additional settings needed for the chosen mode (unused options should be greyed out).

#define usePresets

#if defined(usePresets)
// --- Way 1 to choose the operation mode: choose one of the presets. All further options are automatically set -----------------------------
/* These are the presets:
   Fan mode: speed of the fan will be directly set, either via mqtt, via a touch display or both
   Climate mode: speed of the fan will be controlled by temperature. If actual temperature is higher than target temperature, fan runs at high speed.
     Of course, actual temperature can never be lower than the air temperature which is transported by the fan from "outside" to "inside".
     Actual temperature: the actual temperature can be measured by an BME280 or can be provided via mqtt
     Target temperature: the target temperature will be tried to reach. The target temmperate can be provided via mqtt, via a touch display or both.
*/
// --- Begin: list of presets. Choose exactly one. ---
#define fan_controlledByMQTT
//#define fan_controlledByTouch
//#define fan_controlledByMQTTandTouch
//#define climate_controlledByBME_targetByMQTT
//#define climate_controlledByBME_targetByTouch
//#define climate_controlledByBME_targetByMQTTandTouch
//#define climate_controlledByMQTT_targetByMQTT
//#define climate_controlledByMQTT_targetByMQTTandTouch
// --- End: list of presets --------------------------

// --- based on the preset, automatically define other options --------------
// --- normally you shouldn't change the next lines -------------------------
#if defined(climate_controlledByBME_targetByMQTT) || defined(climate_controlledByBME_targetByTouch) || defined(climate_controlledByBME_targetByMQTTandTouch) || defined(climate_controlledByMQTT_targetByMQTT) || defined(climate_controlledByMQTT_targetByMQTTandTouch)
  #define useAutomaticTemperatureControl
  #if defined(climate_controlledByBME_targetByMQTT) || defined(climate_controlledByBME_targetByTouch) || defined(climate_controlledByBME_targetByMQTTandTouch)
    #define setActualTemperatureViaBME280
    #define useTemperatureSensorBME280
  #endif
  #if defined(climate_controlledByMQTT_targetByMQTT) || defined(climate_controlledByMQTT_targetByMQTTandTouch)
    #define setActualTemperatureViaMQTT
  #endif
#endif
#if defined(fan_controlledByMQTT) || defined(fan_controlledByMQTTandTouch) || defined(climate_controlledByBME_targetByMQTT) || defined(climate_controlledByBME_targetByMQTTandTouch) || defined(climate_controlledByMQTT_targetByMQTT) || defined(climate_controlledByMQTT_targetByMQTTandTouch)
  #define useWIFI
  #define useMQTT
#endif
#if defined(fan_controlledByTouch) || defined(fan_controlledByMQTTandTouch) || defined(climate_controlledByBME_targetByTouch) || defined(climate_controlledByBME_targetByMQTTandTouch) || defined(climate_controlledByMQTT_targetByMQTTandTouch)
  #define useTFT
  #define DRIVER_ILI9341       // e.g. 2.8 inch touch panel, 320x240, used in AZ-Touch
  #define useTouch
#endif

#else
// --- Way 2 to choose the operation mode: manually define every low level option -----------------------------------------------------------
/*
Mode 1: pwm mode
directly setting fan speed via pwm signal
-> comment "useAutomaticTemperatureControl"

Mode 2: temperature controller mode
fan speed automatically increases if temperature is getting close to or higher than target temperature. Of course temperature can never get lower than air temperature of room
-> uncomment "useAutomaticTemperatureControl"
   -> choose "setActualTemperatureViaBME280"
      -> uncomment "useTemperatureSensorBME280"
      XOR "setActualTemperatureViaMQTT"

In both modes you have to have at least one of "useMQTT" and "useTouch", otherwise you cannot control the fan.

Everything else is optional.

First set mode, then go further down in this file to set other options needed for the chosen mode (unused options should be greyed out).

*/
// --- setting mode -------------------------------------------------------------------------------------------------------------------------
// #define useAutomaticTemperatureControl
  #ifdef useAutomaticTemperatureControl
    // --- choose how to set target temperature. Activate only one. --------------------------------------
    #define setActualTemperatureViaBME280
    // #define setActualTemperatureViaMQTT
  #endif
// #define useTemperatureSensorBME280
#define useWIFI
#define useMQTT
// #define useTFT
  #ifdef useTFT
    // --- choose which display to use. Activate only one. -----------------------------------------------
    // #define DRIVER_ILI9341       // 2.8 inch touch panel, 320x240, used in AZ-Touch
    #define DRIVER_ST7735        // 1.8 inch panel,       160x128
  #endif
// #define useTouch
#endif
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- End: choose operation mode ---------------------------------------------------------------------------------------------------------------------------------



// --- Begin: additional settings ---------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
// Now, as the basic operation mode is chosen, you can define additional settings

// --- Home Assistant MQTT discovery --------------------------------------------------------------------------------------------------------
/* If you are using Home Assistant, you can activate auto discovery of the climate/fan and sensors.
   Please also see https://github.com/KlausMu/esp32-fan-controller/wiki/06-Home-Assistant
   If needed, e.g. if you are using more than one esp32 fan controller, please adjust mqtt settings further down in this file */
#if defined(useAutomaticTemperatureControl) && defined(setActualTemperatureViaBME280) && defined(useMQTT)
#define useHomeassistantMQTTDiscovery
#endif
#if defined(useHomeassistantMQTTDiscovery) && (!defined(useAutomaticTemperatureControl) || !defined(setActualTemperatureViaBME280) || !defined(useMQTT))
static_assert(false, "You have to use \"#define useAutomaticTemperatureControl\" and \"#define setActualTemperatureViaBME280\" and \"#define useMQTT\" when having \"#define useHomeassistantMQTTDiscovery\"");
#endif

/* --- If you have a touch display, you can show a standbyButton or shutdownButton
   There are two kind of shutdown buttons:
   1. set the fan controller to standby
        - pwm is set to 0. Note: it is not guaranteed that fan stops if pwm is set to 0
        - display is turned off
        - you can get your fan controller back to normal mode via an mqtt message or if you touch the display
   2. Call an external http REST endpoint, which can trigger any action you want.
      The display of the fan controller simply shows a counter from 30 to 0 and expects something to happen (e.g. power should be turned off by external means).
      If nothing happens and counter reaches 0, the fan controller goes back to normal operation mode.
      For example you could define in Home Assistant an input button. In an automation you could:
        - shutdown a Raspberry Pi
        - turn a smart plug off, so that the Raspberry Pi, a 3D printer and the fan controller get powered off
*/

// #define useStandbyButton
// #define useShutdownButton
#if defined(useStandbyButton) && defined(useShutdownButton)
static_assert(false, "You cannot have both \"#define useStandbyButton\" and \"#define useShutdownButton\"");
#endif

// --- fan specs ----------------------------------------------------------------------------------------------------------------------------
// fanPWM
#define PWMPIN               GPIO_NUM_17
#define PWMFREQ              25000
#define PWMCHANNEL           0
#define PWMRESOLUTION        8
#define FANMAXRPM            1500         // only used for showing at how many percent fan is running

// fanTacho
#define TACHOPIN                             GPIO_NUM_16
#define TACHOUPDATECYCLE                     1000 // how often tacho speed shall be determined, in milliseconds
#define NUMBEROFINTERRUPSINONESINGLEROTATION 2    // Number of interrupts ESP32 sees on tacho signal on a single fan rotation. All the fans I've seen trigger two interrups.

// --- automatic temperature control --------------------------------------------------------------------------------------------------------

// ifdef:  adaptive fan speed depending on actual temperature and target temperature
//         target temperature can be set via tft touch or via mqtt
//         needs "useTemperatureSensorBME280 defined"
// ifndef: fan speed (pwm) is directly set, no adaptive temperature control
//         you can set fan speed either via tft touch or via mqtt

#ifdef useAutomaticTemperatureControl
// initial target temperature on startup
#define INITIALTARGETTEMPERATURE 27.0
// Lowest pwm value the temperature controller should use to set fan speed. If you want the fan not to turn off, set a value so that fan always runs.
#define PWMMINIMUMVALUE            120
#else
// delta used when manually increasing or decreasing pwm
#define PWMSTEP                    10
#endif

// initial pwm fan speed on startup (0 <= value <= 255)
#define INITIALPWMVALUE            120

// sanity check
#if !defined(setActualTemperatureViaBME280) && !defined(setActualTemperatureViaMQTT) && defined(useAutomaticTemperatureControl)
static_assert(false, "You have to use \"#define setActualTemperatureViaBME280\" or \"#define setActualTemperatureViaMQTT\" when having \"#define useAutomaticTemperatureControl\"");
#endif
#if defined(setActualTemperatureViaBME280) && !defined(useTemperatureSensorBME280)
static_assert(false, "You have to use \"#define useTemperatureSensorBME280\" when having \"#define setActualTemperatureViaBME280\"");
#endif
#if defined(setActualTemperatureViaBME280) && defined(setActualTemperatureViaMQTT)
static_assert(false, "You cannot have both \"#define setActualTemperatureViaBME280\" and \"#define setActualTemperatureViaMQTT\"");
#endif

// --- temperature sensor BME280 ------------------------------------------------------------------------------------------------------------

#ifdef useTemperatureSensorBME280
// I2C pins used for BME280
#define I2C_SCL              GPIO_NUM_32 // GPIO_NUM_22 // GPIO_NUM_17
#define I2C_SDA              GPIO_NUM_33 // GPIO_NUM_21 // GPIO_NUM_16
#define I2C_FREQ        100000 // 400000
#define BME280_ADDR      0x76
// in order to calibrate BME280 at startup, provide here the height over sea level in meter at your location
#define HEIGHTOVERSEALEVELATYOURLOCATION 112.0
#endif

// --- wifi ---------------------------------------------------------------------------------------------------------------------------------

#ifdef useWIFI
#define WIFI_SSID     "YourWifiSSID"           // override it in file "config_override.h"
#define WIFI_PASSWORD "YourWifiPassword"       // override it in file "config_override.h"
//#define WIFI_KNOWN_APS_COUNT 2
//#define WIFI_KNOWN_APS \
//  { "00:11:22:33:44:55", "Your AP 2,4 GHz"}, \
//  { "66:77:88:99:AA:BB", "Your AP 5 GHz"}
#endif

// --- OTA Update ---------------------------------------------------------------------------------------------------------------------------
#define useOTAUpdate
// #define useOTA_RTOS     // not recommended because of additional 10K of heap space needed

#if !defined(useWIFI) && defined(useOTAUpdate)
static_assert(false, "\"#define useOTAUpdate\" is only possible with \"#define useWIFI\"");
#endif
#if !defined(ESP32) && defined(useOTA_RTOS)
static_assert(false, "\"#define useOTA_RTOS\" is only possible with ESP32");
#endif
#if defined(useOTA_RTOS) && !defined(useOTAUpdate)
static_assert(false, "You cannot use \"#define useOTA_RTOS\" without \"#define useOTAUpdate\"");
#endif

#define useSerial
#define useTelnetStream

// --- mqtt ---------------------------------------------------------------------------------------------------------------------------------
/*
  ----- IMPORTANT -----
  ----- MORE THAN ONE INSTANCE OF THE ESP32 FAN CONTROLLER -----
  If you want to have more than one instance of the esp32 fan controller in your network, every instance has to have it's own unique mqtt topcics (and IDs in HA, if you are using HA)
  For this the define UNIQUE_DEVICE_NAME is used. You can keep it unchanged if you have only one instance in your network. Otherwise you can change it to e.g. "esp32_fan_controller_1"
*/
#define UNIQUE_DEVICE_NAME     "esp32_fan_controller"

#ifdef useMQTT
#define MQTT_SERVER            "IPAddressOfYourBroker" // override it in file "config_override.h"
#define MQTT_SERVER_PORT       1883                    // override it in file "config_override.h"
#define MQTT_USER              ""                      // override it in file "config_override.h"
#define MQTT_PASS              ""                      // override it in file "config_override.h"
#define MQTT_CLIENTNAME        UNIQUE_DEVICE_NAME

/*
For understanding when "cmnd", "stat" and "tele" is used, have a look at how Tasmota is doing it.
https://tasmota.github.io/docs/MQTT
https://tasmota.github.io/docs/openHAB/
https://www.openhab.org/addons/bindings/mqtt.generic/
https://www.openhab.org/addons/bindings/mqtt/
https://community.openhab.org/t/itead-sonoff-switches-and-sockets-cheap-esp8266-wifi-mqtt-hardware/15024
for debugging:
mosquitto_sub -h localhost -t "esp32_fan_controller/#" -v
mosquitto_sub -h localhost -t "homeassistant/climate/esp32_fan_controller/#" -v
mosquitto_sub -h localhost -t "homeassistant/fan/esp32_fan_controller/#" -v
mosquitto_sub -h localhost -t "homeassistant/sensor/esp32_fan_controller/#" -v
*/

#define MQTTCMNDTARGETTEMP        UNIQUE_DEVICE_NAME "/cmnd/TARGETTEMP"
#define MQTTSTATTARGETTEMP        UNIQUE_DEVICE_NAME "/stat/TARGETTEMP"
#define MQTTCMNDACTUALTEMP        UNIQUE_DEVICE_NAME "/cmnd/ACTUALTEMP"
#define MQTTSTATACTUALTEMP        UNIQUE_DEVICE_NAME "/stat/ACTUALTEMP"
#define MQTTCMNDFANPWM            UNIQUE_DEVICE_NAME "/cmnd/FANPWM"
#define MQTTSTATFANPWM            UNIQUE_DEVICE_NAME "/stat/FANPWM"
// https://www.home-assistant.io/integrations/climate.mqtt/#mode_command_topic
// https://www.home-assistant.io/integrations/climate.mqtt/#mode_state_topic
// note: it is not guaranteed that fan stops if pwm is set to 0
#define MQTTCMNDFANMODE           UNIQUE_DEVICE_NAME "/cmnd/MODE"   // can be "off" and "fan_only"
#define MQTTSTATFANMODE           UNIQUE_DEVICE_NAME "/stat/MODE"
#define MQTTFANMODEOFFPAYLOAD     "off"
#define MQTTFANMODEFANONLYPAYLOAD "fan_only"

#if defined(useOTAUpdate)
#define MQTTCMNDOTA            UNIQUE_DEVICE_NAME "/cmnd/OTA"
#endif

#ifdef useTemperatureSensorBME280
#define MQTTTELESTATE1         UNIQUE_DEVICE_NAME "/tele/STATE1"
#endif
#define MQTTTELESTATE2         UNIQUE_DEVICE_NAME "/tele/STATE2"
#define MQTTTELESTATE3         UNIQUE_DEVICE_NAME "/tele/STATE3"
#define MQTTTELESTATE4         UNIQUE_DEVICE_NAME "/tele/STATE4"

#if defined(useHomeassistantMQTTDiscovery)
/* see
   https://www.home-assistant.io/integrations/mqtt
   https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery
   https://www.home-assistant.io/integrations/mqtt/#discovery-messages
   https://www.home-assistant.io/integrations/mqtt/#birth-and-last-will-messages
*/
#define HASSSTATUSTOPIC                       "homeassistant/status"    // can be "online" and "offline"
#define HASSSTATUSONLINEPAYLOAD               "online"
#define HASSSTATUSOFFLINEPAYLOAD              "offline"
/*
   When HA sends status online, we have to resent the discovery. But we have to wait some seconds, otherwise HA will not recognize the mqtt messages.
   If you have HA running on a weak mini computer, you may have to increase the waiting time. Value is in ms.
   Remark: the whole discovery process will be done in the following order:
   discovery, delay(1000), status=online, delay(1000), all inital values
*/
#define WAITAFTERHAISONLINEUNTILDISCOVERYWILLBESENT   1000

#define HASSCLIMATEDISCOVERYTOPIC             "homeassistant/climate/" UNIQUE_DEVICE_NAME "/config"
#define HASSHUMIDITYSENSORDISCOVERYTOPIC      "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/humidity/config"
#define HASSTEMPERATURESENSORDISCOVERYTOPIC   "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/temperature/config"
#define HASSPRESSURESENSORDISCOVERYTOPIC      "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/pressure/config"
#define HASSALTITUDESENSORDISCOVERYTOPIC      "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/altitude/config"
#define HASSPWMSENSORDISCOVERYTOPIC           "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/pwm/config"
#define HASSRPMSENSORDISCOVERYTOPIC           "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/rpm/config"

// The define HOMEASSISTANTDEVICE will be reused in all discovery payloads for the climate/fan and the sensors. Everything should be contained in the same device.
#define HOMEASSISTANTDEVICE                   "\"dev\":{\"name\":\"Fan Controller\", \"model\":\"" UNIQUE_DEVICE_NAME "\", \"identifiers\":[\"" UNIQUE_DEVICE_NAME "\"], \"manufacturer\":\"KlausMu\"}"
// see https://www.home-assistant.io/integrations/climate.mqtt/
// see https://www.home-assistant.io/integrations/sensor.mqtt/
#define HASSCLIMATEDISCOVERYPAYLOAD           "{\"name\":null,            \"unique_id\":\"" UNIQUE_DEVICE_NAME "\",             \"object_id\":\"" UNIQUE_DEVICE_NAME "\",             \"~\":\"" UNIQUE_DEVICE_NAME "\", \"icon\":\"mdi:fan\", \"min_temp\":10, \"max_temp\":50, \"temp_step\":1, \"precision\":0.1, \"current_humidity_topic\":\"~/tele/STATE1\", \"current_humidity_template\":\"{{value_json.hum | round(0)}}\", \"current_temperature_topic\":\"~/stat/ACTUALTEMP\", \"temperature_command_topic\":\"~/cmnd/TARGETTEMP\", \"temperature_state_topic\":\"~/stat/TARGETTEMP\", \"modes\":[\"off\",\"fan_only\"], \"mode_command_topic\":\"~/cmnd/MODE\", \"mode_state_topic\":\"~/stat/MODE\", \"availability_topic\":\"~/stat/STATUS\", " HOMEASSISTANTDEVICE "}"
#define HASSHUMIDITYSENSORDISCOVERYPAYLOAD    "{\"name\":\"Humidity\",    \"unique_id\":\"" UNIQUE_DEVICE_NAME "_humidity\",    \"object_id\":\"" UNIQUE_DEVICE_NAME "_humidity\",    \"~\":\"" UNIQUE_DEVICE_NAME "\", \"state_topic\":\"~/tele/STATE1\", \"value_template\":\"{{ value_json.hum     | round(0) }}\", \"device_class\":\"humidity\",             \"unit_of_measurement\":\"%\",   \"state_class\":\"measurement\", \"expire_after\": \"30\",                                                                                                                                                                                                                                                                                             " HOMEASSISTANTDEVICE "}"
#define HASSTEMPERATURESENSORDISCOVERYPAYLOAD "{\"name\":\"Temperature\", \"unique_id\":\"" UNIQUE_DEVICE_NAME "_temperature\", \"object_id\":\"" UNIQUE_DEVICE_NAME "_temperature\", \"~\":\"" UNIQUE_DEVICE_NAME "\", \"state_topic\":\"~/tele/STATE1\", \"value_template\":\"{{ value_json.ActTemp | round(1) }}\", \"device_class\":\"temperature\",          \"unit_of_measurement\":\"°C\",  \"state_class\":\"measurement\", \"expire_after\": \"30\",                                                                                                                                                                                                                                                                                             " HOMEASSISTANTDEVICE "}"
#define HASSPRESSURESENSORDISCOVERYPAYLOAD    "{\"name\":\"Pressure\",    \"unique_id\":\"" UNIQUE_DEVICE_NAME "_pressure\",    \"object_id\":\"" UNIQUE_DEVICE_NAME "_pressure\",    \"~\":\"" UNIQUE_DEVICE_NAME "\", \"state_topic\":\"~/tele/STATE1\", \"value_template\":\"{{ value_json.pres    | round(0) }}\", \"device_class\":\"atmospheric_pressure\", \"unit_of_measurement\":\"hPa\", \"state_class\":\"measurement\", \"expire_after\": \"30\",                                                                                                                                                                                                                                                                                             " HOMEASSISTANTDEVICE "}"
#define HASSALTITUDESENSORDISCOVERYPAYLOAD    "{\"name\":\"Altitude\",    \"unique_id\":\"" UNIQUE_DEVICE_NAME "_altitude\",    \"object_id\":\"" UNIQUE_DEVICE_NAME "_altitude\",    \"~\":\"" UNIQUE_DEVICE_NAME "\", \"state_topic\":\"~/tele/STATE1\", \"value_template\":\"{{ value_json.alt     | round(1) }}\", \"device_class\":\"distance\",             \"unit_of_measurement\":\"m\",   \"state_class\":\"measurement\", \"expire_after\": \"30\",                                                                                                                                                                                                                                                                                             " HOMEASSISTANTDEVICE "}"
#define HASSPWMSENSORDISCOVERYPAYLOAD         "{\"name\":\"PWM\",         \"unique_id\":\"" UNIQUE_DEVICE_NAME "_PWM\",         \"object_id\":\"" UNIQUE_DEVICE_NAME "_PWM\",         \"~\":\"" UNIQUE_DEVICE_NAME "\", \"state_topic\":\"~/tele/STATE2\", \"value_template\":\"{{ value_json.pwm }}\",                                                                                            \"state_class\":\"measurement\", \"expire_after\": \"30\",                                                                                                                                                                                                                                                                                             " HOMEASSISTANTDEVICE "}"
#define HASSRPMSENSORDISCOVERYPAYLOAD         "{\"name\":\"RPM\",         \"unique_id\":\"" UNIQUE_DEVICE_NAME "_RPM\",         \"object_id\":\"" UNIQUE_DEVICE_NAME "_RPM\",         \"~\":\"" UNIQUE_DEVICE_NAME "\", \"state_topic\":\"~/tele/STATE2\", \"value_template\":\"{{ value_json.rpm }}\",                                                                                            \"state_class\":\"measurement\", \"expire_after\": \"30\",                                                                                                                                                                                                                                                                                             " HOMEASSISTANTDEVICE "}"

// see https://www.home-assistant.io/integrations/climate.mqtt/#availability_topic
#define HASSFANSTATUSTOPIC                    UNIQUE_DEVICE_NAME "/stat/STATUS" // can be "online" and "offline"
#endif

#endif

// sanity check
#if defined(useMQTT) && !defined(useWIFI)
static_assert(false, "You have to use \"#define useWIFI\" when having \"#define useMQTT\"");
#endif
#if defined(setActualTemperatureViaMQTT) && !defined(useMQTT)
static_assert(false, "You have to use \"#define useMQTT\" when having \"#define setActualTemperatureViaMQTT\"");
#endif

// --- tft ----------------------------------------------------------------------------------------------------------------------------------

#ifdef useTFT
#define TFT_CS                GPIO_NUM_5    //diplay chip select
#define TFT_DC                GPIO_NUM_4    //display d/c
#define TFT_RST               GPIO_NUM_22   //display reset
#define TFT_MOSI              GPIO_NUM_23   //diplay MOSI
#define TFT_CLK               GPIO_NUM_18   //display clock


#ifdef DRIVER_ILI9341
#define TFT_LED               GPIO_NUM_15   //display background LED
#define TFT_MISO              GPIO_NUM_19   //display MISO
#define TFT_ROTATION          3 // use 1 (landscape) or 3 (landscape upside down), nothing else. 0 and 2 (portrait) will not give a nice result.
#endif
#ifdef DRIVER_ST7735
#define TFT_ROTATION          1 // use 1 (landscape) or 3 (landscape upside down), nothing else. 0 and 2 (portrait) will not give a nice result.
#endif

#endif

// --- touch --------------------------------------------------------------------------------------------------------------------------------

// Only AZ-Touch: here you have to set the pin for TOUCH_IRQ. The older "ArduiTouch" and the newer "AZ-Touch" use different pins. And you have to set the LED-PIN to different values to light up the TFT.
// 1. "ArduiTouch" 2.4 inch (older version)
// https://www.az-delivery.de/en/products/az-touch-wandgehauseset-mit-touchscreen-fur-esp8266-und-esp32
#ifdef useTFT
// #define LED_ON           LOW          // override it in file "config_override.h"
#endif
#ifdef useTouch
// #define TOUCH_CS         GPIO_NUM_14  // override it in file "config_override.h"
// #define TOUCH_IRQ        GPIO_NUM_2   // override it in file "config_override.h"
#endif
// 2. "AZ-Touch" 2.8 inch, since November 2020
// https://www.az-delivery.de/en/products/az-touch-wandgehauseset-mit-2-8-zoll-touchscreen-fur-esp8266-und-esp32
// https://www.az-delivery.de/en/blogs/azdelivery-blog-fur-arduino-und-raspberry-pi/az-touch-mod
#ifdef useTFT
#define LED_ON           HIGH          // override it in file "config_override.h"
#endif
#ifdef useTouch
#define TOUCH_CS         GPIO_NUM_14   // override it in file "config_override.h"
#define TOUCH_IRQ        GPIO_NUM_27   // override it in file "config_override.h"
#endif

// sanity check
#if defined(useTouch) && !defined(useTFT)
static_assert(false, "You have to use \"#define useTFT\" when having \"#define useTouch\"");
#endif
#if defined(DRIVER_ST7735) && defined(useTouch)
static_assert(false, "TFT ST7735 doesn't support touch. Please disable it.");
#endif
#if !defined(useTouch) && !defined(useMQTT)
static_assert(false, "You cannot disable both MQTT and touch, otherwise you cannot control the fan");
#endif

// --- shutdown Raspberry Pi and power off --------------------------------------------------------------------------------------------------
/* Shutdown Raspberry Pi and turn off wifi power socket.
   In my setting I have a Raspberry Pi running Octoprint, a 3D printer, and both is powered by a wifi power socket.
   I wanted to shutdown the Pi and turn off power by means of the ESP32.
   This is very special to my setting. You can completely disable it.
   If this option is enabled, then a power off button is shown on the TFT screen.
   When you hit the button, a http request is send to OpenHab which starts a script (script has to be defined in OpenHab) with the following actions:
   - shutdown Raspberry Pi
   - wait 30 seconds
   - turn off wifi power socket (switch off 3D printer and Raspberry Pi)
   Since the OpenHab script (in my case) waits 30 seconds before turning off power, there is a simple countdown with same duration on the TFT display.
   The ESP32 does not need to turn off exactly when 0 is shown on the display. This depends on when the OpenHab script turns off the wifi power socket.
   ESP32 can actually power off when countdown is e.g. at 5 or even less than 0 ...
*/

#ifdef useShutdownButton
#define SHUTDOWNREQUEST                "http://<IPAddressOfYourHAserver:8123>/api/services/input_button/press" // override it in file "config_override.h"
#define SHUTDOWNPAYLOAD                "{\"entity_id\": \"input_button.3dprinter_shutdown\"}"                  // override it in file "config_override.h"
#define SHUTDOWNHEADERNAME1            "Authorization"                                                         // override it in file "config_override.h"
#define SHUTDOWNHEADERVALUE1           "Bearer <your bearer token>"                                            // override it in file "config_override.h"
#define SHUTDOWNHEADERNAME2            "Content-Type"                                                          // override it in file "config_override.h"
#define SHUTDOWNHEADERVALUE2           "application/json"                                                      // override it in file "config_override.h"
#define SHUTDOWNCOUNTDOWN              30  // in seconds
#endif

// sanity check
#if defined(useShutdownButton) && !defined(useWIFI)
static_assert(false, "You have to use \"#define useWIFI\" when having \"#define useShutdownButton\"");
#endif
#if (defined(useStandbyButton) || defined(useShutdownButton)) && !defined(useTouch)
static_assert(false, "You have to use \"#define useTouch\" when having \"#define useStandbyButton\" or \"#define useShutdownButton\"");
#endif

// --- not used -----------------------------------------------------------------------------------------------------------------------------
#ifdef DRIVER_ILI9341
// Occupied by AZ-touch. This software doesn't use this pin
#define BUZZER                GPIO_NUM_21
// #define A0                   GPIO_NUM_36
#endif

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- End: additional settings -----------------------------------------------------------------------------------------------------------------------------------


// --- include override settings from seperate file ---------------------------------------------------------------------------------------------------------------
#if __has_include("config_override.h")
  #include "config_override.h"
#endif

// --- sanity check: only one preset must be choosen --------------------------------------------------------------------------------------------------------------
#if (defined(fan_controlledByMQTT)                         && defined(fan_controlledByTouch))                         || \
    (defined(fan_controlledByMQTT)                         && defined(fan_controlledByMQTTandTouch))                  || \
    (defined(fan_controlledByMQTT)                         && defined(climate_controlledByBME_targetByMQTT))          || \
    (defined(fan_controlledByMQTT)                         && defined(climate_controlledByBME_targetByTouch))         || \
    (defined(fan_controlledByMQTT)                         && defined(climate_controlledByBME_targetByMQTTandTouch))  || \
    (defined(fan_controlledByMQTT)                         && defined(climate_controlledByMQTT_targetByMQTT))         || \
    (defined(fan_controlledByMQTT)                         && defined(climate_controlledByMQTT_targetByMQTTandTouch)) || \
                                                                                                                         \
    (defined(fan_controlledByTouch)                        && defined(fan_controlledByMQTTandTouch))                  || \
    (defined(fan_controlledByTouch)                        && defined(climate_controlledByBME_targetByMQTT))          || \
    (defined(fan_controlledByTouch)                        && defined(climate_controlledByBME_targetByTouch))         || \
    (defined(fan_controlledByTouch)                        && defined(climate_controlledByBME_targetByMQTTandTouch))  || \
    (defined(fan_controlledByTouch)                        && defined(climate_controlledByMQTT_targetByMQTT))         || \
    (defined(fan_controlledByTouch)                        && defined(climate_controlledByMQTT_targetByMQTTandTouch)) || \
                                                                                                                         \
    (defined(fan_controlledByMQTTandTouch)                 && defined(climate_controlledByBME_targetByMQTT))          || \
    (defined(fan_controlledByMQTTandTouch)                 && defined(climate_controlledByBME_targetByTouch))         || \
    (defined(fan_controlledByMQTTandTouch)                 && defined(climate_controlledByBME_targetByMQTTandTouch))  || \
    (defined(fan_controlledByMQTTandTouch)                 && defined(climate_controlledByMQTT_targetByMQTT))         || \
    (defined(fan_controlledByMQTTandTouch)                 && defined(climate_controlledByMQTT_targetByMQTTandTouch)) || \
                                                                                                                         \
    (defined(climate_controlledByBME_targetByMQTT)         && defined(climate_controlledByBME_targetByTouch))         || \
    (defined(climate_controlledByBME_targetByMQTT)         && defined(climate_controlledByBME_targetByMQTTandTouch))  || \
    (defined(climate_controlledByBME_targetByMQTT)         && defined(climate_controlledByMQTT_targetByMQTT))         || \
    (defined(climate_controlledByBME_targetByMQTT)         && defined(climate_controlledByMQTT_targetByMQTTandTouch)) || \
                                                                                                                         \
    (defined(climate_controlledByBME_targetByTouch)        && defined(climate_controlledByBME_targetByMQTTandTouch))  || \
    (defined(climate_controlledByBME_targetByTouch)        && defined(climate_controlledByMQTT_targetByMQTT))         || \
    (defined(climate_controlledByBME_targetByTouch)        && defined(climate_controlledByMQTT_targetByMQTTandTouch)) || \
                                                                                                                         \
    (defined(climate_controlledByBME_targetByMQTTandTouch) && defined(climate_controlledByMQTT_targetByMQTT))         || \
    (defined(climate_controlledByBME_targetByMQTTandTouch) && defined(climate_controlledByMQTT_targetByMQTTandTouch)) || \
                                                                                                                         \
    (defined(climate_controlledByMQTT_targetByMQTT)        && defined(climate_controlledByMQTT_targetByMQTTandTouch))
static_assert(false, "You cannot choose more than one preset at the same time");
#endif

#endif /*__CONFIG_H__*/

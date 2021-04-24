#include <driver/gpio.h>
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
 #define useAutomaticTemperatureControl
  #ifdef useAutomaticTemperatureControl
    // --- choose how to set target temperature. Activate only one. --------------------------------------
  //  #define setActualTemperatureViaBME280
    #define setActualTemperatureViaDHT
   //  #define setActualTemperatureViaMQTT
  #endif
//#define useTemperatureSensorBME280
#define useTemperatureSensorDHT
#define useWIFI
#define useMQTT
// #define useTFT
  #ifdef useTFT
    // --- choose which display to use. Activate only one. -----------------------------------------------
   //  #define DRIVER_ILI9341       // 2.8 inch touch panel, 320x240, used in AZ-Touch
    #define DRIVER_ST7735        // 1.8 inch panel,       160x128
  #endif
// #define useTouch
// #define showShutdownButton

// --- fan specs ----------------------------------------------------------------------------------------------------------------------------
// fanPWM
const int pwmPin               = GPIO_NUM_17;
const int pwmFreq              = 25000;
const int pwmChannel           = 0;
const int pwmResolution        = 8;
const int fanMaxRPM            = 1500;         // only used for showing at how many percent fan is running

// fanTacho
const int tachoPin                             = GPIO_NUM_16;
const int tachoUpdateCycle                     = 1000; // how often tacho speed shall be determined, in milliseconds
const int numberOfInterrupsInOneSingleRotation = 2;    // Number of interrupts ESP32 sees on tacho signal on a single fan rotation. All the fans I've seen trigger two interrups.

// --- automatic temperature control --------------------------------------------------------------------------------------------------------

// ifdef:  adaptive fan speed depending on actual temperature and target temperature
//         target temperature can be set via tft touch or via mqtt
//         needs "useTemperatureSensorBME280 = true"
// ifndef: fan speed (pwm) is directly set, no adaptive temperature control
//         you can set fan speed either via tft touch or via mqtt

#ifdef useAutomaticTemperatureControl
// initial target temperature on startup
const float initialTargetTemperature = 27.0;
// Lowest pwm value the temperature controller should use to set fan speed. If you want the fan not to turn off, set a value so that fan always runs.
const int pwmMinimumValue            = 120;
#else
// delta used when manually increasing or decreasing pwm
const int pwmStep                    = 10;
#endif

// initial pwm fan speed on startup (0 <= value <= 255)
const int initialPwmValue            = 120;

// sanity check
#if !defined(setActualTemperatureViaDHT) &&!defined(setActualTemperatureViaBME280) && !defined(setActualTemperatureViaMQTT) && defined(useAutomaticTemperatureControl)
static_assert(false, "You have to use \"#define setActualTemperatureViaBME280\" or \"#define setActualTemperatureViaMQTT\" or \"#define setActualTemperatureViaDHT\" when having \"#define useAutomaticTemperatureControl\"");
#endif
#if defined(setActualTemperatureViaBME280) && !defined(useTemperatureSensorBME280)
static_assert(false, "You have to use \"#define useTemperatureSensorBME280\" when having \"#define setActualTemperatureViaBME280\"");
#endif
#if defined(setActualTemperatureViaDHT) && !defined(useTemperatureSensorDHT)
static_assert(false, "You have to use \"#define useTemperatureSensorDHT\" when having \"#define setActualTemperatureViaDHT\"");
#endif
#if defined(setActualTemperatureViaBME280) && defined(setActualTemperatureViaMQTT)
static_assert(false, "You cannot have both \"#define setActualTemperatureViaBME280\" and \"#define setActualTemperatureViaMQTT\"");
#endif
#if defined(setActualTemperatureViaDHT) && defined(setActualTemperatureViaMQTT)
static_assert(false, "You cannot have both \"#define setActualTemperatureViaDHT\" and \"#define setActualTemperatureViaMQTT\"");
#endif
// --- temperature sensor BME280 ------------------------------------------------------------------------------------------------------------

#ifdef useTemperatureSensorBME280
// I2C pins used for BME280
const int I2C_SCL              = GPIO_NUM_32; // GPIO_NUM_22; // GPIO_NUM_17
const int I2C_SDA              = GPIO_NUM_33; // GPIO_NUM_21; // GPIO_NUM_16
const int I2C_FREQ             = 100000; // 400000
const uint8_t BME280_addr      = 0x76;
// in order to calibrate BME280 at startup, provide here the height over sea level in meter at your location
const float heightOverSealevelAtYourLocation = 112.0;
#endif

// --- temperature sensor DHT ------------------------------------------------------------------------------------------------------------

#ifdef useTemperatureSensorDHT
// Dé-commentez la ligne qui correspond à votre capteur 
  // DHT 11 
  // DHT 22  (AM2302)
  // DHT 21 (AM2301)

//#define DHTTYPE DHT11     // DHT 11 
 #define DHTTYPE DHT22      // DHT 22  (AM2302)
//#define DHTTYPE DHT21     // DHT 21 (AM2301)

const int DHTPIN               =   GPIO_NUM_4;  // Changer le pin sur lequel est branché le DHT

#endif

// --- wifi ---------------------------------------------------------------------------------------------------------------------------------

#ifdef useWIFI
#endif

// --- mqtt ---------------------------------------------------------------------------------------------------------------------------------

#ifdef useMQTT
/*
For understanding when "cmnd", "stat" and "tele" is used, have a look at how Tasmota is doing it.
https://tasmota.github.io/docs/MQTT
https://tasmota.github.io/docs/openHAB/
https://www.openhab.org/addons/bindings/mqtt.generic/
https://www.openhab.org/addons/bindings/mqtt/
https://community.openhab.org/t/itead-sonoff-switches-and-sockets-cheap-esp8266-wifi-mqtt-hardware/15024
for debugging:
mosquitto_sub -h localhost -t "esp32_fan_controller/#" -v
*/
const char* const mqttCmndTargetTemp     = "esp32_fan_controller/cmnd/TARGETTEMP";
const char* const mqttStatTargetTemp     = "esp32_fan_controller/stat/TARGETTEMP";
const char* const mqttCmndActualTemp     = "esp32_fan_controller/cmnd/ACTUALTEMP";
const char* const mqttStatActualTemp     = "esp32_fan_controller/stat/ACTUALTEMP";
const char* const mqttCmndFanPWM         = "esp32_fan_controller/cmnd/FANPWM";
const char* const mqttStatFanPWM         = "esp32_fan_controller/stat/FANPWM";

#ifdef useTemperatureSensorBME280
const char* const mqttTeleState1         = "esp32_fan_controller/tele/STATE1";
#endif
#ifdef useTemperatureSensorDHT
const char* const mqttTeleState1         = "esp32_fan_controller/tele/STATE1";
#endif
const char* const mqttTeleState2         = "esp32_fan_controller/tele/STATE2";
const char* const mqttTeleState3         = "esp32_fan_controller/tele/STATE3";

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
const int TFT_CS                = GPIO_NUM_5 ;   //diplay chip select
const int TFT_DC                = GPIO_NUM_4 ;   //display d/c
const int TFT_RST               = GPIO_NUM_22;   //display reset
const int TFT_MOSI              = GPIO_NUM_23;   //diplay MOSI
const int TFT_CLK               = GPIO_NUM_18;   //display clock


#ifdef DRIVER_ILI9341
const int TFT_LED               = GPIO_NUM_15;   //display background LED
const int TFT_MISO              = GPIO_NUM_19;   //display MISO
const int TFT_rotation          = 3; // use 1 (landscape) or 3 (landscape upside down), nothing else. 0 and 2 (portrait) will not give a nice result.
#endif
#ifdef DRIVER_ST7735
const int TFT_rotation          = 1; // use 1 (landscape) or 3 (landscape upside down), nothing else. 0 and 2 (portrait) will not give a nice result.
#endif

#endif

// --- touch --------------------------------------------------------------------------------------------------------------------------------

#ifdef useTouch
const int TOUCH_CS              = GPIO_NUM_14;
// Only AZ-Touch: here you have to set the pin for TOUCH_IRQ. The older "ArduiTouch" and the newer "AZ-Touch" use different pins
// use GPIO_NUM_2 for the older "ArduiTouch" 2.4 inch
// https://www.az-delivery.de/en/products/az-touch-wandgehauseset-mit-touchscreen-fur-esp8266-und-esp32
const int TOUCH_IRQ             = GPIO_NUM_2 ;   // touch screen interrupt
// use GPIO_NUM_27 for the newer "AZ-Touch" 2.8 inch, since November 2020
// https://www.az-delivery.de/en/products/az-touch-wandgehauseset-mit-2-8-zoll-touchscreen-fur-esp8266-und-esp32
// https://www.az-delivery.de/en/blogs/azdelivery-blog-fur-arduino-und-raspberry-pi/az-touch-mod
// const int TOUCH_IRQ          = GPIO_NUM_27 ;   // touch screen interrupt
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

#ifdef showShutdownButton
const char* const shutdownRequest  = "http://<IPAddressOfYourOpenHABserver:Port>/rest/items/Shutdown3DPrinter";
const char* const shutdownPayload  = "ON";
const int shutdownCountdown        = 30;  // in seconds
#endif

// sanity check
#if defined(showShutdownButton) && !defined(useWIFI)
static_assert(false, "You have to use \"#define useWIFI\" when having \"#define showShutdownButton\"");
#endif
#if defined(showShutdownButton) && !defined(useTouch)
static_assert(false, "You have to use \"#define useTouch\" when having \"#define showShutdownButton\"");
#endif

// --- not used -----------------------------------------------------------------------------------------------------------------------------
#ifdef DRIVER_ILI9341
// Occupied by AZ-touch. This software doesn't use this pin
const int BUZZER                = GPIO_NUM_21;
// const int A0                   = GPIO_NUM_36;
#endif
# ESP32 fan controller with MQTT support
This project describes how to use an ESP32 microcontroller for controlling a 4 pin fan (pwm controlled fan). Main features are:
* mode 1 (pwm mode): directly setting fan speed via pwm signal
* mode 2 (temperature controller mode): fan speed automatically increases if temperature is getting close to or higher than target temperature. Of course temperature can never get lower than air temperature of room.
* measurement of fan speed via tacho signal
* measurement of ambient values via BME280: temperature, humidity, pressure
* support of MQTT
* support of OTA (over the air updates of firmware). Please see <a href="https://github.com/KlausMu/esp32-fan-controller/wiki/06-OTA---Over-the-air-updates">Wiki: 06 OTA Over the air updates</a>

* TFT display for showing status information, different resolutions supported (tested with 320x240 and 160x128)
* TFT touch display for setting pwm or target temperature
* optional: integration into home automation software <a href="https://www.openhab.org/">openHAB</a>

Even if you don't want to use all of these features, the project can hopefully easily be simplified or extended. With some minor modifications an ESP8266 / D1 mini should be usable.

I did this project for having an automatic temperature controller for my 3D printer housing. But of course at least the ideas used here could be used for many other purposes.

<b>For more information please see the <a href="https://github.com/KlausMu/esp32-fan-controller/wiki">Wiki</a></b>

## Operation modes
You can operate the ESP32 fan controller mainly in two different modes, depending on your needs:
mode | description | how to set PWM | how to set actual temperature | how to set target temperature
------------ | ------------- | ------------- | ------------- | -------------
pwm mode | fan speed directly set via PWM signal | MQTT, touch or both | BME280 (optional, only used for information) |
temperature controller mode | automatic temperature control<br>fan speed is automatically set depending on difference between target temperature and actual temperature | | MQTT or BME280 | MQTT, touch or both

In both modes, a TFT panel can optionally be used for showing status information from the fan, ambient (BME280: temperature, humidity, pressure) and the chosen target temperature. Different resolutions of the TFT panel are supported, layout will automatically be adapted (tested with 320x240 and 160x128).

If you use a TFT touch panel, you can set the PWM value or target temperature via the touch panel (otherwise you have to use MQTT).

<b>For more information please see the <a href="https://github.com/KlausMu/esp32-fan-controller/wiki/03-Examples:-operation-modes-and-breadboards">Wiki: 03 Examples: operation modes and breadboards</a></b>

## Wiring diagram for fan and BME280
![Wiring diagram fan and BME280](https://github.com/KlausMu/esp32-fan-controller/wiki/images/fritzingESP32_BME280_fan.png)

<b>For more information please see the <a href="https://github.com/KlausMu/esp32-fan-controller/wiki/01-Wiring-diagram">Wiki: 01 Wiring diagram</a></b>

## Part list
Function | Parts | Remarks | approx. price
------------ | ------------- | ------------- | -------------
<b>mandatory</b>
microcontroller | ESP32 | e.g. from  <a href="https://www.az-delivery.de/en/products/esp32-developmentboard">AZ-Delivery</a> | 8 EUR
fan | 4 pin fan (4 pin means pwm controlled), 5V or 12V | tested with a standard CPU fan and a Noctua NF-F12 PWM<br>for a list of premium fans see https://noctua.at/en/products/fan | 20 EUR for Noctua
measuring tacho signal of fan | - pullup resistor 10 k&#8486;<br>- RC circuit: resistor 3.3 k&#8486;; ceramic capacitor 100 pF
power supply | - 5V for ESP32, 5V or 12V for fan (depending on fan)<br>or<br>-12V when using AZ-touch (see below) | e.g. with 5.5×2.5 mm coaxial power connector | 12 EUR
<b>optional</b>
temperature sensor | - BME280<br>- 2 pullup resistors 3.3 k&#8486; (for I2C) | e.g. from  <a href="https://az-delivery.de/en/products/gy-bme280">AZ-Delivery</a> | 6.50 EUR
<b>optional</b>
TFT display (non touch) | 1.8 inch 160x128, ST7735 | e.g. from  <a href="https://www.az-delivery.de/en/products/1-8-zoll-spi-tft-display">AZ-Delivery</a> | 6.80 EUR
TFT touch display with ESP32 housing | AZ-touch from AZ delivery<br>including voltage regulator and TFT touch display (2.8 inch 320x240, ILI9341, XPT2046) | e.g. from  <a href="https://www.az-delivery.de/en/products/az-touch-wandgehauseset-mit-2-8-zoll-touchscreen-fur-esp8266-und-esp32">AZ-Delivery</a> <br>(you can also use the older <a href="https://www.az-delivery.de/en/products/az-touch-wandgehauseset-mit-touchscreen-fur-esp8266-und-esp32">2.4 inch ArduiTouch</a>)| 30 EUR
connectors for detaching parts from AZ-touch | - e.g. 5.5×2.5 mm coaxial power connector male<br>- JST-XH 2.54 mm for BME280<br>- included extra cables and connectors in case of Noctua fan

Other TFTs can most likely easily be used, as long as there is a library from Adafruit for it. If resolution is smaller than 160x128 it might be necessary to change the code in file tft.cpp. Anything bigger should automatically be rearranged. If you want to use touch, your TFT should have the XPT2046 chip to use it without any code change.

## Software installation
If you're only used to the Arduino IDE, I highly recommend having a look at <a href="https://platformio.org/">PlatformIO IDE</a>.

While the Arduino IDE is sufficient for flashing, it is not very comfortable for software development. There is no syntax highlighting and no autocompletion. All the needed libraries have to be installed manually, and you will sooner or later run into trouble with different versions of the same library.

This cannot happen with <a href="https://platformio.org/">PlatformIO</a>. All libraries will automatically be installed into the project folder and cannot influence other projects.

If you absolutely want to use the Arduino IDE, please have look at the file "platformio.ini" for the libraries needed.

For installing PlatformIO IDE, follow this <a href="https://docs.platformio.org/en/latest/integration/ide/vscode.html#installation">guide</a>. It is as simple as:
* install VSCode (Visual Studio Code)
* install PlatformIO as an VSCode extension
* clone this repository or download it
* use "open folder" in VSCode to open this repository
* check settings in "config.h"
* upload to ESP32

## Images
### ArduiTouch running in "temperature controller mode"
![TempControllerModeArduiTouch](https://github.com/KlausMu/esp32-fan-controller/wiki/images/tempControllerModeArduiTouch.jpg)
### Images of ESP32 fan controller used in a 3D printer housing
<!--- [[images/overview_esp32.jpg \| width=600px]] -->
<!--- ![ArduiTouch](https://github.com/KlausMu/esp32-fan-controller/wiki/images/overview_esp32.jpg | width=600) doesn't work -->
![3DPrinter](https://github.com/KlausMu/esp32-fan-controller/wiki/images/3Dprinter.jpg)

<b>For more information please see the <a href="https://github.com/KlausMu/esp32-fan-controller/wiki/04-AZ%E2%80%90touch-or-ArduiTouch">Wiki: 04 AZ‐touch / ArduiTouch</a></b>

#include <XPT2046_Touchscreen.h>
#include <TouchEvent.h>

#include "config.h"
#include "fanPWM.h"
#include "log.h"
#include "shutdownRaspPi.h"
#include "temperatureController.h"
#include "tft.h"

#ifdef useTouch
//prepare driver for touch screen
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
// init TouchEvent with pointer to the touch screen driver
TouchEvent tevent(touch);

// point on touchscreen that got hit
TS_Point p;
// current position
int tsx, tsy, tsxraw, tsyraw;

// checks if point x/y is inside rect[]
bool pointInRect(const int rect[], int x, int y) {
  if (TFT_rotation == 3) {
    return
      (x >= rect[0]) &&
      (x <= rect[0] + rect[2]) &&
      (y >= rect[1]) &&
      (y <= rect[1] + rect[3]);
  } else if (TFT_rotation == 1) {
    return
      (tft_getWidth()  - x >= rect[0]) &&
      (tft_getWidth()  - x <= rect[0] + rect[2]) &&
      (tft_getHeight() - y >= rect[1]) &&
      (tft_getHeight() - y <= rect[1] + rect[3]);
  }
}

void onClick(TS_Point p) {
  // store x and y as raw data
  tsxraw = p.x;
  tsyraw = p.y;

  tsx = 320 - tsxraw;
  tsy = 240 - tsyraw;
  Log.printf("click %d %d\r\n", tsx, tsy);

  if (screen == SCREEN_NORMALMODE) {
    if (pointInRect(valueUpRect, tsx, tsy)) {
      Log.printf("up button hit\r\n");
      #ifdef useAutomaticTemperatureControl
        updatePWM_MQTT_Screen_withNewTargetTemperature(getTargetTemperature() + 1, true);
      #else
        incFanSpeed();
      #endif
    } else if (pointInRect(valueDownRect, tsx, tsy)) {
      Log.printf("down button hit\r\n");
      #ifdef useAutomaticTemperatureControl
        updatePWM_MQTT_Screen_withNewTargetTemperature(getTargetTemperature() -1, true);
      #else
        decFanSpeed();
      #endif
    #ifdef showShutdownButton
    }  else if (pointInRect(shutdownRect, tsx, tsy)) {
      Log.printf("shutdown button hit\r\n");
      screen = SCREEN_CONFIRMSHUTDOWN;
      // clear screen
      tft_fillScreen();
      draw_screen();
    #endif
    }
  #ifdef showShutdownButton
  } else if (screen == SCREEN_CONFIRMSHUTDOWN) {
    if (pointInRect(confirmShutdownYesRect, tsx, tsy)) {
      Log.printf("confirm shutdown yes hit\r\n");
      if (shutdownRaspPi()){
        screen = SCREEN_COUNTDOWN;
        startCountdown = millis();
      } else {
        screen =SCREEN_NORMALMODE;
      }
      // clear screen
      tft_fillScreen();
      draw_screen();
    } else if (pointInRect(confirmShutdownNoRect, tsx, tsy)) {
      Log.printf("confirm shutdown no hit\r\n");
      screen = SCREEN_NORMALMODE;
      // clear screen
      tft_fillScreen();
      draw_screen();
    }
  #endif
  }
}
#endif
void initTFTtouch(void) {
  #ifdef useTouch
  //start driver
  touch.begin();

  //init TouchEvent instance
  tevent.setResolution(tft_getWidth(),tft_getHeight());
  tevent.setDblClick(010);
//  tevent.registerOnTouchSwipe(onSwipe);
  tevent.registerOnTouchClick(onClick);
//  tevent.registerOnTouchDblClick(onDblClick);
//  tevent.registerOnTouchLong(onLongClick);
//  tevent.registerOnTouchDraw(onDraw);
//  tevent.registerOnTouchDown(onTouch);
//  tevent.registerOnTouchUp(onUntouch);

  Log.printf("  TFTtouch sucessfully initialized.\r\n");
  #else
  Log.printf("    Touch is disabled in config.h\r\n");
  #endif
}

void processUserInput(void) {
  #ifdef useTouch
  tevent.pollTouchScreen();
  #endif
}
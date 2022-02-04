#include "config.h"
#include "fanPWM.h"
#include "fanTacho.h"
#include "log.h"
#include "sensorBME280.h"
#include "temperatureController.h"
#include "tft.h"

#ifdef DRIVER_ILI9341
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeSans9pt7b.h>
#endif
#ifdef DRIVER_ST7735
#include <Adafruit_ST7735.h>
#endif

//prepare driver for display
#ifdef DRIVER_ILI9341
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
#endif
#ifdef DRIVER_ST7735
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
// Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);
#endif

#ifdef useTFT
const GFXfont *myFont;
int textSizeOffset;

// number of screen to display
int screen = SCREEN_NORMALMODE;

unsigned long startCountdown = 0;

void calcDimensionsOfElements(void);
void draw_screen(void);
#endif

void initTFT(void) {
  #ifdef useTFT
  // start driver
  #ifdef DRIVER_ILI9341
  // switch display on
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, LED_ON);
  tft.begin();
  myFont = &FreeSans9pt7b;
  textSizeOffset = 0;
  #endif
  #ifdef DRIVER_ST7735
  tft.initR(INITR_BLACKTAB);
  myFont = NULL;
  textSizeOffset = 0;
  #endif

  tft.setFont(myFont);
  tft.setRotation(TFT_rotation); 

  calcDimensionsOfElements();

  // clear screen
  tft.fillScreen(TFT_BLACK);
  draw_screen();

  // show the displays resolution
  Log.printf("  TFT sucessfully initialized.\r\n");
  Log.printf("  tftx = %d, tfty = %d\r\n", tft.width(), tft.height());

  #else
  Log.printf("    TFT is disabled in config.h\r\n");
  #endif
}

#ifdef useTFT
int16_t getRelativeX(int16_t xBasedOnTFTwithScreenWidth320px) {
  return (float)(xBasedOnTFTwithScreenWidth320px) /(float)(320) * tft_getWidth();;
}

int16_t getRelativeY(int16_t yBasedOnTFTwithScreenHeight240px) {
  return (float)(yBasedOnTFTwithScreenHeight240px)/(float)(240) * tft_getHeight();;
}

// rect: x, y, width, heigth
int valueUpRect[4];
int valueDownRect[4];
#ifdef showShutdownButton
int shutdownRect[4];
int confirmShutdownYesRect[4];
int confirmShutdownNoRect[4];
#endif

int plusMinusHorizontalLineMarginLeft;
int plusMinusHorizontalLineMarginTop;
int plusMinusHorizontalLineLength;
int plusMinusVerticalLineMarginTop;
int plusMinusVerticalLineLength;
int plusMinusVerticalLineMarginLeft;

int tempAreaLeft; int tempAreaTop; int tempAreaWidth;
int fanAreaLeft; int fanAreaTop; int fanAreaWidth;
int ambientAreaLeft; int ambientAreaTop; int ambientAreaWidth;

#ifdef showShutdownButton
int shutdownWidthAbsolute;
int shutdownHeightAbsolute;
#endif

void calcDimensionsOfElements(void) {
  // upper left corner is 0,0
  // width and heigth are only valid for landscape (rotation=1) or landscape upside down (rotation=3)
  //                       ILI9341   ST7735
  //                       AZ-Touch
  // tft.width   0 <= x <  320       160
  // tft.height  0 <= y <  240       128

  // ALL VALUES ARE BASED ON A 320x240 DISPLAY and automatically resized to the actual display size via getRelativeX() and getRelativeYI()
  int marginTopAbsolute   = 12;
  int marginLeftAbsolute  = 14;
  // int areaHeightAbsolute  = 64;           // make sure: 4*marginTopAbsolute + 3*areaHeightAbsolute = 240
  int areaHeightAbsolute  = (240 - 4*marginTopAbsolute) / 3;

  int valueUpDownWidthAbsolute  = 80;
  int valueUpDownHeightAbsolute = 55;
  #ifdef showShutdownButton
      shutdownWidthAbsolute     = 40;
      shutdownHeightAbsolute    = 40;
  #endif
  int valueUpRectTop;
  int valueDownRectTop;
  #ifdef showShutdownButton
  int shutdownRectTop;
  #endif

  tempAreaLeft     = getRelativeX(marginLeftAbsolute);
  fanAreaLeft      = getRelativeX(marginLeftAbsolute);
  ambientAreaLeft = getRelativeX(marginLeftAbsolute);
  #ifdef useAutomaticTemperatureControl
  tempAreaTop     = getRelativeY(marginTopAbsolute);
  fanAreaTop      = getRelativeY(marginTopAbsolute+areaHeightAbsolute+marginTopAbsolute);
  valueUpRectTop   = fanAreaTop;
  ambientAreaTop = getRelativeY(marginTopAbsolute+areaHeightAbsolute+marginTopAbsolute+areaHeightAbsolute+marginTopAbsolute );
  valueDownRectTop = ambientAreaTop;
    #ifdef showShutdownButton
    tempAreaWidth     = getRelativeX(320-marginLeftAbsolute - shutdownWidthAbsolute-marginLeftAbsolute);    // screen - marginleft - [Area] - 40 shutdown - marginright
    #else
    tempAreaWidth     = getRelativeX(320-marginLeftAbsolute -    0);                                        // screen - marginleft - [Area]               - marginright
    #endif
    #ifdef useTouch
    fanAreaWidth      = getRelativeX(320-marginLeftAbsolute - valueUpDownWidthAbsolute-marginLeftAbsolute); // screen - marginleft - [Area] - 80 up/down  - marginright
    ambientAreaWidth = getRelativeX(320-marginLeftAbsolute - valueUpDownWidthAbsolute-marginLeftAbsolute);
    #else
    fanAreaWidth      = getRelativeX(320-marginLeftAbsolute -    0);                                        // screen - marginleft - [Area]               - marginright
    ambientAreaWidth = getRelativeX(320-marginLeftAbsolute -    0);
    #endif
    #ifdef showShutdownButton
    shutdownRectTop   = getRelativeY(marginTopAbsolute);
    #endif
  #else
  fanAreaTop      = getRelativeY(marginTopAbsolute);
  valueUpRectTop    = fanAreaTop;
  ambientAreaTop = getRelativeY(marginTopAbsolute+areaHeightAbsolute+marginTopAbsolute);
  valueDownRectTop  = ambientAreaTop;
    #ifdef useTouch
    fanAreaWidth      = getRelativeX(320-marginLeftAbsolute - valueUpDownWidthAbsolute-marginLeftAbsolute); // screen - marginleft - [Area] - 80 up/down  - marginright
    ambientAreaWidth = getRelativeX(320-marginLeftAbsolute - valueUpDownWidthAbsolute-marginLeftAbsolute);
    #else
    fanAreaWidth      = getRelativeX(320-marginLeftAbsolute -    0);                                        // screen - marginleft - [Area]               - marginright
    ambientAreaWidth = getRelativeX(320-marginLeftAbsolute -    0);
    #endif
    #ifdef showShutdownButton
    shutdownRectTop   = getRelativeY(240-shutdownHeightAbsolute-marginTopAbsolute);
    #endif
  #endif

  valueUpRect[0] = getRelativeX(320-valueUpDownWidthAbsolute-marginLeftAbsolute);
  valueUpRect[1] = valueUpRectTop;
  valueUpRect[2] = getRelativeX(valueUpDownWidthAbsolute);
  valueUpRect[3] = getRelativeY(valueUpDownHeightAbsolute);

  valueDownRect[0] = getRelativeX(320-valueUpDownWidthAbsolute-marginLeftAbsolute);
  valueDownRect[1] = valueDownRectTop;
  valueDownRect[2] = getRelativeX(valueUpDownWidthAbsolute);
  valueDownRect[3] = getRelativeY(valueUpDownHeightAbsolute);

  plusMinusHorizontalLineLength     = (valueUpRect[2] / 2) - 4; // 36
  plusMinusHorizontalLineMarginLeft = (valueUpRect[2] - plusMinusHorizontalLineLength) / 2; // 22
  plusMinusHorizontalLineMarginTop  = valueUpRect[3] / 2; // 27

  plusMinusVerticalLineLength       = plusMinusHorizontalLineLength; // 36
  plusMinusVerticalLineMarginTop    = (valueUpRect[3] - plusMinusVerticalLineLength) / 2; // 9
  plusMinusVerticalLineMarginLeft   = valueUpRect[2] / 2; // 40

  #ifdef showShutdownButton
  shutdownRect[0] = getRelativeX(320-shutdownWidthAbsolute-marginLeftAbsolute);
  shutdownRect[1] = shutdownRectTop;
  shutdownRect[2] = getRelativeX(shutdownWidthAbsolute);
  shutdownRect[3] = getRelativeY(shutdownHeightAbsolute);

  confirmShutdownYesRect[0]  = getRelativeX(40);
  confirmShutdownYesRect[1]  = getRelativeY(90);
  confirmShutdownYesRect[2]  = getRelativeX(60);
  confirmShutdownYesRect[3]  = getRelativeY(60);
  confirmShutdownNoRect[0]   = getRelativeX(200);
  confirmShutdownNoRect[1]   = getRelativeY(90);
  confirmShutdownNoRect[2]   = getRelativeX(60);
  confirmShutdownNoRect[3]   = getRelativeY(60);
  #endif
}

int16_t tft_getWidth(void) {
  return tft.width();
}

int16_t tft_getHeight(void) {
  return tft.height();
}

void tft_fillScreen(void) {
  tft.fillScreen(TFT_BLACK);
};

/*
https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
                        AZ-Touch
              ST7735    ILI9341
  TextSize    Standard  FreeSans9pt7b FreeSerif9pt7b  FreeMono9pt7b  FreeSerifBoldItalic24pt7b
              y1/h
  1 pwm hum   0/8       -12/17        -11/16          -9/13          -30/40
  2 temp      0/16      -24/34        -22/32          -18/26         -60/80
  3           0/24      -36/51        -33/48          -27/39         -90/120
  4 countdown 0/32      -48/68        -44/64          -36/52         -120/160
  8           0/64      -96/136       -88/128         -72/104        -240/320
  15          0/120     -180/255      -165240         -135/195       -450/600
*/
void printText(int areaX, int areaY, int areaWidth, int lineNr, const char *str, uint8_t textSize, const GFXfont *f, bool wipe) {
  // get text bounds
  GFXcanvas1 testCanvas(tft_getWidth(), tft_getHeight());
  int16_t x1; int16_t y1; uint16_t w; uint16_t h;
  testCanvas.setFont(f);
  testCanvas.setTextSize(textSize);
  testCanvas.setTextWrap(false);
  testCanvas.getTextBounds("0WIYgy,", 0, 0, &x1, &y1, &w, &h);
  // Log.printf("  x1 = %d, y1 = %d, w=%d, h=%d\r\n", x1, y1, w, h);
  int textHeight = h;
  int textAreaHeight = textHeight +2; // additional 2 px as vertical spacing between lines
  // y1=0 only for standardfont, with every other font this value gets negative!
  // This means that when using standarfont at (0,0), it really gets printed at 0,0.
  // With every other font, printing at (0,0) means that text starts at (0, y1) with y1 being negative!
  int textAreaOffset = -y1;
  // Don't know why, but a little additional correction has to be done for every font other than standard font. Doesn't work perfectly, sometimes position is wrong by 1 pixel
  // if (textAreaOffset != 0) {
  //   textAreaOffset = textAreaOffset + textSize;
  // }

  // Version 1: flickering, but faster
  #ifdef useTouch
  tft.setFont(f);
  tft.setTextSize(textSize);
  tft.setTextWrap(false);
  if (wipe) {
    tft.fillRect (areaX, areaY + lineNr*textAreaHeight, areaWidth, textAreaHeight, TFT_BLACK);
  }
  tft.setCursor(areaX, areaY + textAreaOffset + lineNr*textAreaHeight);
  tft.printf(str);
  #endif

  // Version 2: flicker-free, but slower. Touch becomes unusable.
  #ifndef useTouch
  GFXcanvas1 canvas(areaWidth, textAreaHeight);
  canvas.setFont(f);
  canvas.setTextSize(textSize);
  canvas.setTextWrap(false);
  canvas.setCursor(0, textAreaOffset);
  canvas.println(str);
  tft.drawBitmap(areaX, areaY + lineNr*textAreaHeight, canvas.getBuffer(), areaWidth, textAreaHeight, TFT_WHITE, TFT_BLACK);
  #endif
}
#endif

void draw_screen(void) {
  #ifdef useTFT
  char buffer[100];
  // don't understand why I have to do this
  #ifdef useTouch
  String percentEscaped = "%%";
  #else
  String percentEscaped = "%";
  #endif

  if (screen == SCREEN_NORMALMODE) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // actual temperature and target temperature
    #ifdef useAutomaticTemperatureControl
    sprintf(buffer, "%.1f (actual)", getActualTemperature());
    printText(tempAreaLeft, tempAreaTop, tempAreaWidth, 0, buffer, textSizeOffset + 2, myFont, true);
    sprintf(buffer, "%.1f (target)", getTargetTemperature());
    printText(tempAreaLeft, tempAreaTop, tempAreaWidth, 1, buffer, textSizeOffset + 2, myFont, true);
    #endif
  
    // fan
    printText(fanAreaLeft, fanAreaTop, fanAreaWidth, 0, "Fan:", textSizeOffset + 1, myFont, false);
    sprintf(buffer, "%d rpm (%d%s)", last_rpm, (100*last_rpm)/fanMaxRPM, percentEscaped.c_str());
    printText(fanAreaLeft, fanAreaTop, fanAreaWidth, 1, buffer, textSizeOffset + 1, myFont, true);
    sprintf(buffer, "%d/255 pwm (%d%s)", getPWMvalue(), (100*getPWMvalue())/255, percentEscaped.c_str());
    printText(fanAreaLeft, fanAreaTop, fanAreaWidth, 2, buffer, textSizeOffset + 1, myFont, true);

    // relative humidity, barometric pressure and ambient temperature
    #ifdef useTemperatureSensorBME280
    int ambientLine = 0;
    printText(ambientAreaLeft, ambientAreaTop, ambientAreaWidth, ambientLine++, "Ambient:", textSizeOffset + 1, myFont, false);
    #if   ( (!defined(useAutomaticTemperatureControl) && defined(useTemperatureSensorBME280)) || ( defined(useAutomaticTemperatureControl) && defined(setActualTemperatureViaMQTT)) )
    sprintf(buffer, "%.1f temperature", lastTempSensorValues[0]);
    printText(ambientAreaLeft, ambientAreaTop, ambientAreaWidth, ambientLine++, buffer,      textSizeOffset + 1, myFont, true);
    #endif
    sprintf(buffer, "%.2f hPa (%.2f m)", lastTempSensorValues[1], lastTempSensorValues[2]);
    printText(ambientAreaLeft, ambientAreaTop, ambientAreaWidth, ambientLine++, buffer,      textSizeOffset + 1, myFont, true);
    sprintf(buffer, "%.2f%s humidity", lastTempSensorValues[3], percentEscaped.c_str());
    printText(ambientAreaLeft, ambientAreaTop, ambientAreaWidth, ambientLine++, buffer,      textSizeOffset + 1, myFont, true);
    #endif
  
    #ifdef useTouch
    // increase temperature or pwm
    tft.fillRoundRect(valueUpRect[0],   valueUpRect[1],    valueUpRect[2],   valueUpRect[3],   4, TFT_GREEN);
    //   plus sign
    //     horizontal line
    tft.drawLine(valueUpRect[0]+plusMinusHorizontalLineMarginLeft, valueUpRect[1]+plusMinusHorizontalLineMarginTop,   valueUpRect[0]+plusMinusHorizontalLineMarginLeft + plusMinusHorizontalLineLength, valueUpRect[1]+plusMinusHorizontalLineMarginTop,   TFT_BLACK);
    tft.drawLine(valueUpRect[0]+plusMinusHorizontalLineMarginLeft, valueUpRect[1]+plusMinusHorizontalLineMarginTop+1, valueUpRect[0]+plusMinusHorizontalLineMarginLeft + plusMinusHorizontalLineLength, valueUpRect[1]+plusMinusHorizontalLineMarginTop+1, TFT_BLACK);
    //     vertical line
    tft.drawLine(valueUpRect[0]+plusMinusVerticalLineMarginLeft,   valueUpRect[1]+plusMinusVerticalLineMarginTop, valueUpRect[0]+plusMinusVerticalLineMarginLeft,   valueUpRect[1]+plusMinusVerticalLineMarginTop + plusMinusVerticalLineLength, TFT_BLACK);
    tft.drawLine(valueUpRect[0]+plusMinusVerticalLineMarginLeft+1, valueUpRect[1]+plusMinusVerticalLineMarginTop, valueUpRect[0]+plusMinusVerticalLineMarginLeft+1, valueUpRect[1]+plusMinusVerticalLineMarginTop + plusMinusVerticalLineLength, TFT_BLACK);
    // decrease temperature or pwm
    tft.fillRoundRect(valueDownRect[0], valueDownRect[1],  valueDownRect[2], valueDownRect[3], 4, TFT_GREEN);
    //   minus sign
    //     horizontal line
    tft.drawLine(valueDownRect[0]+plusMinusHorizontalLineMarginLeft, valueDownRect[1]+plusMinusHorizontalLineMarginTop,   valueDownRect[0]+plusMinusHorizontalLineMarginLeft + plusMinusHorizontalLineLength, valueDownRect[1]+plusMinusHorizontalLineMarginTop,   TFT_BLACK);
    tft.drawLine(valueDownRect[0]+plusMinusHorizontalLineMarginLeft, valueDownRect[1]+plusMinusHorizontalLineMarginTop+1, valueDownRect[0]+plusMinusHorizontalLineMarginLeft + plusMinusHorizontalLineLength, valueDownRect[1]+plusMinusHorizontalLineMarginTop+1, TFT_BLACK);
    #endif

    #ifdef showShutdownButton
    // shutdown button
    // square, without round corners
    // tft.fillRect(shutdownRect[0],   shutdownRect[1],    shutdownRect[2],   shutdownRect[3],   TFT_RED);
    // round corners, inner part
    tft.fillRoundRect(shutdownRect[0], shutdownRect[1], shutdownRect[2], shutdownRect[3], getRelativeX(4), TFT_RED);
    // round corners, outer line
    // tft.drawRoundRect(shutdownRect[0], shutdownRect[1], shutdownRect[2], shutdownRect[3], 4, TFT_WHITE);
    tft.drawCircle(shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2),   shutdownRect[1]+getRelativeY(shutdownHeightAbsolute/2), getRelativeX(shutdownWidthAbsolute*0.375),   TFT_WHITE);
    tft.drawCircle(shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2),   shutdownRect[1]+getRelativeY(shutdownHeightAbsolute/2), getRelativeX(shutdownWidthAbsolute*0.375)-1, TFT_WHITE);
    tft.drawLine(  shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2),   shutdownRect[1]+getRelativeY(shutdownHeightAbsolute/4), shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2),   shutdownRect[1]+getRelativeY(shutdownHeightAbsolute*3/4), TFT_WHITE);
    tft.drawLine(  shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2)+1, shutdownRect[1]+getRelativeY(shutdownHeightAbsolute/4), shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2)+1, shutdownRect[1]+getRelativeY(shutdownHeightAbsolute*3/4), TFT_WHITE);
    #endif
  #ifdef showShutdownButton
  } else if (screen == SCREEN_CONFIRMSHUTDOWN) {
    printText(getRelativeX(44), getRelativeY(50), getRelativeX(200), 0, "Please confirm shutdown",      textSizeOffset + 1, myFont, false);

    // confirm: yes
    tft.fillRoundRect(confirmShutdownYesRect[0], confirmShutdownYesRect[1], confirmShutdownYesRect[2], confirmShutdownYesRect[3], 4, TFT_RED);
    printText(confirmShutdownYesRect[0]+getRelativeX(12), confirmShutdownYesRect[1] + getRelativeY(22), getRelativeX(200), 0, "Yes",      textSizeOffset + 1, myFont, false);
    // confirm: no
    tft.fillRoundRect(confirmShutdownNoRect[0],  confirmShutdownNoRect[1],  confirmShutdownNoRect[2],  confirmShutdownNoRect[3],  4, TFT_GREEN);
    printText(confirmShutdownNoRect[0]+ getRelativeX(18), confirmShutdownNoRect[1]  + getRelativeY(22), getRelativeX(200), 0, "No",       textSizeOffset + 1, myFont, false);
  } else if (screen == SCREEN_COUNTDOWN) {
    float floatSecondsSinceShutdown = (unsigned long)(millis()-startCountdown) / 1000;
    // rounding
    floatSecondsSinceShutdown = floatSecondsSinceShutdown + 0.5 - (floatSecondsSinceShutdown<0);
    // convert float to int
    int intSecondsSinceShutdown = (int)floatSecondsSinceShutdown;

    // clear screen
    tft.fillScreen(TFT_BLACK);
    sprintf(buffer, "%d", shutdownCountdown - intSecondsSinceShutdown);
    printText(getRelativeX(115), getRelativeY(80), getRelativeX(200), 0, buffer,       textSizeOffset + 4, myFont, false);

    if ((unsigned long)(millis() - startCountdown) > shutdownCountdown*1000 + 15000){
      // if EPS32 is still alive, which means power is still available, then stop countdown and go back to normal mode
      Log.printf("hm, still alive? Better show mainscreen again\r\n");
      screen = SCREEN_NORMALMODE;
      // clear screen
      tft.fillRect(0, 0, 320, 240, TFT_BLACK);
      draw_screen();
    }
  #endif
  }
  #endif
}

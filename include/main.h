#ifndef MAIN_H
#define MAIN_H
#define FASTLED_ESP8266_D1_PIN_ORDER

#include <FastLED.h>

#define USE_LEDS
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#ifdef USE_LEDS
#define NUM_LEDS 1
#define DATA_PIN D4
#define COLOR_ORDER GRB
#define MILLI_AMPS 2000

#define FRAMES_PER_SECOND 120

CRGB leds[NUM_LEDS];
#endif


uint8_t gHue = 0; // rotating "base color" used by many of the patterns
char nameChar[12];
uint8_t brightness, currentPatternIndex, power;
CRGB solidColor;
uint8_t gCurrentPaletteNumber = 0;





void prepareWifi(void);
void prepareWebServer(char* nameChar);
void prepareOTA(String nameString);

void loadSettings(void);

void setPower(uint8_t value);
void sendString(String value);
void sendInt(uint8_t value);
void setSolidColor(CRGB color);
void setSolidColor(uint8_t r, uint8_t g, uint8_t b);
void setPattern(uint8_t value);
void showSolidColor(void);



typedef void (*Pattern)();
typedef Pattern PatternList[];
typedef struct {
  Pattern pattern;
  String name;
} PatternAndName;
typedef PatternAndName PatternAndNameList[];


#include "patterns.h"
PatternAndNameList patterns = {
   { showSolidColor,         "Solid Color" },
   { rainbow, "Rainbow"}
};

const uint8_t patternCount = ARRAY_SIZE(patterns);

#endif
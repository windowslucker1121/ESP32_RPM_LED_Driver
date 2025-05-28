#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include "Settings.h"
#include <vector>
#include <FastLED.h>
#include "Pins.h"

#define LED_TYPE    WS2812 
#define COLOR_ORDER GRB

class LedDriver {
public:
    LedDriver(int numLeds, Settings*& settings);
    void Setup();
    void loop(bool log);
    void SetInterval(int interval);
    
private:
    std::vector<bool> ShowLedState(int value);
    void playStartupAnimation();
    int startupAnimationMilliSeconds = 3000;
    int numLeds;
    int dataPin;
    Settings*& settings;
    int updateInterval = 300;
    unsigned long lastUpdate = 0;
    int brightness;
    CRGB * leds;
};

#endif // LED_DRIVER_H
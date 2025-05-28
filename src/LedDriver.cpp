#include "LedDriver.h"
#include "LeftToRightStyle.h"
#include "EqualFromBothSidesStyle.h"
#include <Arduino.h>
#include "UniLog.h"
#include "ColorSchema.h"

const int bootDelayPerLed = 100;
const int flashingAmount = 10;

LedDriver::LedDriver(int numLeds, Settings*& settings)
    : numLeds(numLeds), settings(settings) {
        leds = new CRGB[numLeds];
}

void LedDriver::Setup() {
    if (settings->getDisplayStyle() == nullptr || settings->getIsEnabled() == false) {
        logger.logln("Display style is not set or LED Driver is disabled by user setting. Skipping LED setup.");
        return;
    }

    logger.logln("LED Driver setup with " + String(numLeds) + " LEDs on pin " + String(LED_DRIVER_PIN));
    FastLED.addLeds<LED_TYPE, LED_DRIVER_PIN, COLOR_ORDER>(leds, numLeds)
    .setCorrection(TypicalLEDStrip)
    .setDither(DISABLE_DITHER);
    
    brightness = settings->getLedBrightness();
    FastLED.setBrightness(brightness);
    playStartupAnimation();

}

void LedDriver::SetInterval(int interval) {
    this->updateInterval = interval;
}

void LedDriver::loop(bool log) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdate >= updateInterval) {
        lastUpdate = currentMillis;
    } else {
        return; 
    }

    if (settings->getLedBrightness() != brightness) {
        brightness = settings->getLedBrightness();
        FastLED.setBrightness(brightness);
        logger.logln("LED brightness updated to: " + String(brightness));
    }

    if (settings->getIsEnabled() == false) {
        ShowLedState(0);
        return;
    }

    int value = settings->getCurrentMappedValue();
    if (value == -1) {
        logger.logln("Invalid value from PWM Reader, skipping LED update.");
        return;
    }
    if (value < 0) value = 0;
    if (value > 100) value = 100;

    std::vector<bool> states = ShowLedState(value);

    if (log)
    {
        logger.log("LED States: ");
        for (int i = 0; i < numLeds; ++i) {
            logger.log(String(states[i]) + " ");
        }
        logger.log("\n");
    }

}

std::vector<bool> LedDriver::ShowLedState(int value)
{
    if (value < 0) value = 0;
    if (value > 100) value = 100;

    // logger.logln("value: " + String(value) + ", numLeds: " + String(numLeds));
    if (numLeds <= 0) {
        logger.logln("Number of LEDs is not set or invalid. Skipping LED update.");
        return std::vector<bool>();
    }
    std::vector<bool> ledStates(numLeds, 0);

    if (leds == nullptr) {
        logger.logln("LEDs are not initialized. Skipping LED update.");
        return ledStates;
    }

    // logger.logln("Initialized ledStates with size: " + String(ledStates.size()));
    if (settings == nullptr )
    {
        logger.logln("Settings are not initialized. Skipping LED update.");
        return ledStates;
    }
    ILedDisplayStyle * displayStyle = settings->getDisplayStyle();
    if (displayStyle == nullptr) {
        logger.logln("Display style is not set -> something is off here. Skipping...");
        return ledStates;
    }
    // logger.logln("Display style: " + displayStyle->getName());
    // logger.logln("Getting LED states for value: " + String(value) + " using display style: " + displayStyle->getName());
    displayStyle->getLedStates(value, ledStates);
    // logger.logln("Led states retrieved from display style: " + String(ledStates.size()));
    ColorSchema colorSchema = settings->getDisplayStyle()->getColorSchema();

    // logger.logln(colorSchema.ToString());

    if (ledStates.size() != numLeds) {
        logger.logln("LED states size does not match number of LEDs. Skipping LED update.");
        return ledStates;
    }

    for (int i = 0; i < numLeds;) {
        if (ledStates[i]) {
            if (value <= colorSchema.lowPercent) {
                leds[i] = colorSchema.lowColor;
            } else if (value >= 100 - colorSchema.highPercent) {
                leds[i] = colorSchema.highColor;
            } else {
                leds[i] = colorSchema.midColor;
            }
        } else {
            leds[i] = CRGB::Black;
        }
        i++;
    }

    FastLED.show();
    return ledStates;
}


void LedDriver::playStartupAnimation() {
    //200 steps because we want 0 to 100 and from 100 to 0
    logger.logln("Playing startup animation...");
    int animTime = settings->getBootupAnimationTime();

    if (animTime <= 0) {
        logger.logln("Invalid animation time: " + String(animTime) + ". Using default 3000ms.");
        animTime = 3000; 
    }
    int delayPerStep = animTime / 200;

    for (int i = 0; i <= 100; i++) {
        // logger.logln("LED value: " + String(i) + "calling showledState");
        ShowLedState(i);
        delay(delayPerStep);
    }

    for (int i = 100; i >= 0; i--) {
        ShowLedState(i);
        delay(delayPerStep);
    }
    logger.logln("Startup animation completed.");
}
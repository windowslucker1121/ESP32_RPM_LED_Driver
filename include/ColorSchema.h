#ifndef COLOR_SCHEMA_H
#define COLOR_SCHEMA_H

#include <FastLED.h>

class ColorSchema {
public:
    CRGB lowColor = CRGB::Green;
    CRGB midColor = CRGB::Yellow;
    CRGB highColor = CRGB::Red;
    uint8_t lowPercent = 40;
    uint8_t highPercent = 30;

    String ToString() const {
        return String("ColorSchema(lowColor: RGB(") + lowColor.r + ", " + lowColor.g + ", " + lowColor.b +
               "), midColor: RGB(" + midColor.r + ", " + midColor.g + ", " + midColor.b +
               "), highColor: RGB(" + highColor.r + ", " + highColor.g + ", " + highColor.b +
               "), lowPercent: " + String(lowPercent) +
               ", highPercent: " + String(highPercent) + ")";
    }
};

#endif
// CustomLedStyle.h
#ifndef CUSTOM_LED_STYLE_H
#define CUSTOM_LED_STYLE_H

#include "ILedDisplayStyle.h"
#include <vector>
#include <Arduino.h>
#include "ArduinoJson.h"

enum StylePattern {
    LEFT_TO_RIGHT = 0,
    RIGHT_TO_LEFT = 1,
    CENTER_TO_EDGES = 2,
    EDGES_TO_CENTER = 3
};

class CustomLedStyle : public ILedDisplayStyle {
private:
    String name;
    StylePattern pattern;
    ColorSchema colorSchema;
    
    bool enableFlashing;
    int flashInterval;
    
    bool flashState = false;
    unsigned long lastFlashToggle = 0;

public:
    CustomLedStyle(const String& name, StylePattern pattern, 
                  const ColorSchema& colors, bool enableFlashing = false, 
                  int flashInterval = 100);
    
    void getLedStates(int value, std::vector<bool>& ledStates) override;
    String getName() override { return name; }
    ColorSchema getColorSchema() override { return colorSchema; }
    
    String toJson() const;
    static CustomLedStyle* fromJson(const String& json);
};

#endif // CUSTOM_LED_STYLE_H

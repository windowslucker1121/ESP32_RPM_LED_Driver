#ifndef FLASHING_LEFT_TO_RIGHT_STYLE_H
#define FLASHING_LEFT_TO_RIGHT_STYLE_H

#include "ILedDisplayStyle.h"
#include <vector>
#include <stdint.h>
#include <Arduino.h>

class FlashingLeftToRightStyle : public ILedDisplayStyle {
private:
    bool flashState = false;
    unsigned long lastFlashToggle = 0;
    const unsigned long flashInterval = 80;

public:
    void getLedStates(int value, std::vector<bool>& ledStates) override;
    String getName() override;
};

#endif // FLASHING_LEFT_TO_RIGHT_STYLE_H

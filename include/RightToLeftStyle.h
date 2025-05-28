#ifndef RIGHT_TO_LEFT_STYLE_H
#define RIGHT_TO_LEFT_STYLE_H

#include "ILedDisplayStyle.h"
#include <vector>
#include <stdint.h>
#include <Arduino.h>

class RightToLeftStyle : public ILedDisplayStyle {
public:
    void getLedStates(int value, std::vector<bool>& ledStates) override;
    String getName() override;
};

#endif // RIGHT_TO_LEFT_STYLE_H
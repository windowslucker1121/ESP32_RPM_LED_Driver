#ifndef LEFT_TO_RIGHT_STYLE_H
#define LEFT_TO_RIGHT_STYLE_H

#include "ILedDisplayStyle.h"
#include <vector>
#include <stdint.h>
#include <Arduino.h>

class LeftToRightStyle : public ILedDisplayStyle {
public:
    void getLedStates(int value, std::vector<bool>& ledStates) override;
    String getName() override;
};

#endif // LEFT_TO_RIGHT_STYLE_H
#ifndef EQUAL_FROM_BOTH_SIDES_STYLE_H
#define EQUAL_FROM_BOTH_SIDES_STYLE_H
#include <Arduino.h>
#include "ILedDisplayStyle.h"
#include <vector>

class EqualFromBothSidesStyle : public ILedDisplayStyle {
public:
    EqualFromBothSidesStyle();
    void getLedStates(int value, std::vector<bool>& ledStates) override;
    String getName() override;
};

#endif // EQUAL_FROM_BOTH_SIDES_STYLE_H
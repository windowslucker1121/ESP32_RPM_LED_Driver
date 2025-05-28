#include "EqualFromBothSidesStyle.h"
#include <cmath>
#include "ColorSchema.h"

EqualFromBothSidesStyle::EqualFromBothSidesStyle() {
    colorSchema.lowColor = CRGB::Blue;
    colorSchema.midColor = CRGB::Purple;
    colorSchema.highColor = CRGB::Red;
}

void EqualFromBothSidesStyle::getLedStates(int value, std::vector<bool>& ledStates) {
    int numLeds = ledStates.size();
    int activeLeds = std::round((value / 100.0) * numLeds);
    int halfActive = activeLeds / 2;

    for (int i = 0; i < numLeds; ++i) {
        ledStates[i] = false;
    }

    for (int i = 0; i < halfActive; ++i) {
        ledStates[i] = 1;
        ledStates[numLeds - 1 - i] = true;
    }

    if (activeLeds % 2 != 0 && activeLeds == numLeds) {
        ledStates[numLeds / 2] = true;
    }
}

String EqualFromBothSidesStyle::getName() {
    return "EqualFromBothSides";
}
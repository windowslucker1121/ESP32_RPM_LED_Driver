#include "FlashingLeftToRightStyle.h"
#include "ColorSchema.h"

void FlashingLeftToRightStyle::getLedStates(int value, std::vector<bool>& ledStates) {
    const int numLeds = ledStates.size();
    int numLedsToLight = (value * numLeds) / 100;
    if (value % 10 >= 5 && numLedsToLight < numLeds) {
        numLedsToLight++;
    }
    const unsigned long currentTime = millis();
    int flashBorder = 100 - colorSchema.highPercent;

    if (value >= flashBorder) {
        if (currentTime - lastFlashToggle >= flashInterval) {
            flashState = !flashState;
            lastFlashToggle = currentTime;
        }
    } else {
        flashState = true; 
    }

    for (int i = 0; i < numLeds; ++i) {
        const bool shouldLight = i < numLedsToLight;
        ledStates[i] = (shouldLight && (value < flashBorder || flashState)) ? true : false;
    }
}

String FlashingLeftToRightStyle::getName() {
    return "FlashingLeftToRight";
}

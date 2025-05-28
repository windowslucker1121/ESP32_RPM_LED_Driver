#include "RightToLeftStyle.h"

void RightToLeftStyle::getLedStates(int value, std::vector<bool>& ledStates) {
    int numLedsToLight = (value * ledStates.size()) / 100;

    if (value % 10 >= 5 && numLedsToLight < ledStates.size()) {
        numLedsToLight++;
    }

    for (size_t i = 0; i < ledStates.size(); ++i) {
        ledStates[i] = (i >= ledStates.size() - numLedsToLight) ? true : false;
    }
}

String RightToLeftStyle::getName() {
    return "RightToLeft";
}
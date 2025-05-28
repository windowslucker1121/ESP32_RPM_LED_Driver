#ifndef PWM_GENERATOR_H
#define PWM_GENERATOR_H

#include <Arduino.h>
#include "Settings.h"

class PWMGenerator {
private:
    int frequency;
    unsigned long startTime;
    unsigned long lastFrequencyChange;
    unsigned long refreshInterval;
    int CalcResolutionBits();
    Settings*& settings;

public:
    PWMGenerator(Settings*& settings);
    void setup();
    void loop(bool log);
    void SetInterval(int interval);
};

#endif // PWM_GENERATOR_H
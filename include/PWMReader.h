#ifndef PWM_READER_H
#define PWM_READER_H

#include <Arduino.h>
#include "ISensor.h"
#include "Settings.h"

class PWMReader : public ISensor {
private:
    volatile unsigned long pulseStart;
    volatile unsigned long pulseWidth;
    volatile unsigned long period = 0;
    volatile unsigned long lastRise = 0;
    int lastInterval = 0;
    int refreshInterval = 1000;
    int lastValue = 0;
    
public:
    PWMReader(Settings*& settings);
    void setup() override;
    int readValue(bool log) override;
    int readRawValue();
    void SetInterval(int interval);
private:
    static void IRAM_ATTR handleInterrupt(void* arg);
    Settings*& settings;

    int readerMin;
    int readerMax;
};

#endif // PWM_READER_H
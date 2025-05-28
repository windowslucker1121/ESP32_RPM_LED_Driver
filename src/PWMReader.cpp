#include "PWMReader.h"
#include "UniLog.h"
#include "Pins.h"

PWMReader::PWMReader(Settings *& settings) : pulseStart(0), pulseWidth(0), settings(settings) {
}

void PWMReader::setup() {
    pinMode(PWM_READER_PIN, INPUT_PULLDOWN);
    attachInterruptArg(digitalPinToInterrupt(PWM_READER_PIN), handleInterrupt, this, CHANGE);
    readerMin = settings->getReaderMinFrequency();
    readerMax = settings->getReaderMaxFrequency();
}

void PWMReader::SetInterval(int interval) {
    this->refreshInterval = interval;
}

int PWMReader::readRawValue()
{
    noInterrupts();
    unsigned long currentPeriod = period;
    interrupts();
    if (currentPeriod == 0) return 0;
    float frequency = 1e6 / currentPeriod;
    if (frequency > 10000000) {
        logger.logln("Frequency unplausible, returning 0%");
        settings->setCurrentAnalogValue(-1);
        return -1;
    }

    frequency = constrain(frequency, readerMin, readerMax);

    static float filteredFreq = frequency;
    filteredFreq = 0.7 * filteredFreq + 0.3 * frequency;

    return filteredFreq;
}

int PWMReader::readValue(bool log) {
    if (millis() - lastInterval > refreshInterval) {
        lastInterval = millis();
    } else {
        return lastValue;
    }

    if (settings->getReaderMinFrequency() != readerMin || settings->getReaderMaxFrequency() != readerMax) {
        readerMin = settings->getReaderMinFrequency();
        readerMax = settings->getReaderMaxFrequency();
        logger.logln("Reader min/max frequencies updated: " + String(readerMin) + "/" + String(readerMax));
    }

    if (readerMin == -1 || readerMax == -1) {
        logger.logln("Frequency range not set, returning 0%");
        return -1;
    }

    float frequency = readRawValue();

    int mappedValue = map(static_cast<int>(frequency), readerMin, readerMax, 0, 100);
    mappedValue = constrain(mappedValue, 0, 100);

    if (log) {
        logger.logln("PWM Reader (min/max/mappedProgress): " + String(readerMin) + "/" + String(readerMax) + "/" + String(mappedValue) + " PWM Value: " + String(frequency));
    }

    lastValue = mappedValue;
    settings->setCurrentAnalogValue(frequency);
    settings->setCurrentMappedValue(mappedValue);
    return mappedValue;
}



void IRAM_ATTR PWMReader::handleInterrupt(void* arg) {
    PWMReader* instance = static_cast<PWMReader*>(arg);
    if (digitalRead(PWM_READER_PIN) == HIGH) {
        unsigned long now = micros();
        if (instance->lastRise != 0) {
            instance->period = now - instance->lastRise;
        }
        instance->lastRise = now;
        instance->pulseStart = now;
    } else {
        instance->pulseWidth = micros() - instance->pulseStart;
    }
}

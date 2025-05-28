#include "PWMGenerator.h"
#include "UniLog.h"
#include "Pins.h"
#include <Arduino.h>

PWMGenerator::PWMGenerator(Settings*& settings) : settings(settings) {
    frequency = settings->getGeneratorMinFrequency();
    startTime = 0; 
    lastFrequencyChange = millis(); 
}

void PWMGenerator::setup() {
    pinMode(PWM_GENERATOR_PIN, OUTPUT);
    ledcAttach(PWM_GENERATOR_PIN, frequency, CalcResolutionBits());
    ledcWrite(PWM_GENERATOR_PIN, 128); 
    lastFrequencyChange = startTime;
}

void PWMGenerator::SetInterval(int interval) {
    this->refreshInterval = interval;
}
void PWMGenerator::loop(bool log) {
    unsigned long currentTime = millis();
    
    if (currentTime - lastFrequencyChange < refreshInterval) {
        return;
    }
    
    unsigned long elapsed = currentTime - lastFrequencyChange;
    lastFrequencyChange = currentTime;
    startTime += elapsed;

    int minFrequency = settings->getGeneratorMinFrequency();
    int maxFrequency = settings->getGeneratorMaxFrequency();
    int roundTime = settings->getRoundTimeGenerator();

    float progress = (float)startTime / roundTime;
    frequency = minFrequency + (maxFrequency - minFrequency) * progress;

    if (startTime >= roundTime) {
        startTime = 0;
    }

    ledcChangeFrequency(PWM_GENERATOR_PIN, frequency, CalcResolutionBits());
    ledcWrite(PWM_GENERATOR_PIN, 128);
    
    if (log) {
        logger.logln("PWM Generator (min/max/current) and (progress): " 
                   + String(minFrequency) + "/" + String(maxFrequency) + "/" 
                   + String(frequency) + "    (" + String(progress) + ")");
    }
}

int PWMGenerator::CalcResolutionBits() {
    int new_resolution = 14;
    if(frequency > 1000) new_resolution = 6;
    else if(frequency > 100) new_resolution = 10;
    // logger.logln("Resolution: " + String(new_resolution) + " bits" + " for frequency: " + String(frequency) + " Hz");
    return new_resolution;
}
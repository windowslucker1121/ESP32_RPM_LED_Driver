#include "DevSensor.h"

void DevSensor::setup() {
    pinMode(3, INPUT);
}

int DevSensor::readValue(bool log) {
    int value = analogRead(3);
    value = map(value, 0, 4090, 0, 100);
    return value;
}
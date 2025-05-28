#ifndef DEV_SENSOR_H
#define DEV_SENSOR_H

#include <Arduino.h>
#include "ISensor.h"

class DevSensor : public ISensor {
public:
    void setup() override;
    int readValue(bool log) override;
};

#endif // DEV_SENSOR_H
// File: ISensor.h
#ifndef ISENSOR_H
#define ISENSOR_H

#include <Arduino.h>

class ISensor {
public:
    virtual void setup() = 0; 
    virtual int readValue(bool log) = 0; 
    virtual void SetInterval(int interval) {}
    virtual ~ISensor() {} 
};

#endif // ISENSOR_H
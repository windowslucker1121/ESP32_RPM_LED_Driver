#pragma once

#if defined(USE_S3_DEVKIT_PINS)
#define LED_DRIVER_PIN 10
#define PWM_GENERATOR_PIN 8
#define PWM_READER_PIN 46
#define LED_COUNT 8
#elif defined(USE_S3_ZERO_PINS)
#define LED_DRIVER_PIN 12
#define PWM_GENERATOR_PIN 3
#define PWM_READER_PIN 10
#define LED_COUNT 24
#endif
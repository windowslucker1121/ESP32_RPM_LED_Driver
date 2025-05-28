
# Configureable RPM LED Motorcycle Display Device ESP32

Welcome to my small hobby project which i did out of curiosity. 
This Project aims to solve my issue where i moved from a ZX6R which has a higher max RPM border then my new 2017 Z1000SX. 
This fact gets me in situations where i find myself in hitting the Rev Limiter, because i couldnt really notice the "small" Shift indicator provided by Z1000SX.

Enough words here is the finished "Product":
![](https://github.com/windowslucker1121/ESP32_RPM_LED_Driver/blob/main/pictures/finished.PNG?raw=true)

#### Details:

I am by no means a Electrical Engineer just to be clear.

In general this whole thing works with an ESP32 which receives the PWM Signal from the Speedometer. 
Then looks up the configured Values and maps that to the LEDs, everything is customizeable, so it should also fit non Kawasaki Speedometers. 
The minimum required thing is that the Speedometer Signal can be extracted from somewhere and is an PWM Signal. 
You may could add another Sensor, by implementing [ISensor](https://github.com/windowslucker1121/ESP32_RPM_LED_Driver/blob/main/include/ISensor.h) to fit other types of Readings aswell.

## Current features of the program


* Show Realtime Information for PWM Signal
* Show Realtime Information for Mapped PWM Signal (0-100)
* Configure
  * Enabled / Disabled
  * MinMHz = all Leds off
  * MaxHZ = all Leds on
  * Brightness
  * Current Animation Style
  * Bootup Animation Duration
* Choose from 1 of 4 default Led Styles
* Create Custom Led Styles (Colors,Flashing, Animation Direction)
* Remote Logger via Webserial
* Remote Flash (no need to extract the esp32 if you want to update)
## Demo
![](https://github.com/windowslucker1121/ESP32_RPM_LED_Driver/blob/main/pictures/Animation.gif?raw=true)

## TODO
 -  electrical schema
 - used parts
 - explain setup and platform io (2 setups, one for development and one for the mounted device)

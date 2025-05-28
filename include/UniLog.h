#ifndef UNILOG_H
#define UNILOG_H

#include <Arduino.h>
#include <WebSerial.h>

class UniLog {
public:
    enum LogMode {
        SERIAL_MODE,
        WIFI,
        BOTH
    };
    bool isEnabled = true;

    void begin(LogMode mode, AsyncWebServer* server = nullptr) {
        this->mode = mode;
        Serial.println("Logger initialized in mode: " + String(mode == SERIAL_MODE ? "SERIAL" : "WIFI"));
        if (( mode == WIFI || mode == BOTH ) && server != nullptr) {
            Serial.println("Initializing WebSerial...");
            WebSerial.begin(server);
            Serial.println("WebSerial initialized.");
            Serial.println("Server beginning...");
            server->begin();
            Serial.println("Server started.");
            webSerialInitialized = true;
        }
    }

    void log(const String& message) {
        if (!isEnabled) return;
        if (mode == SERIAL_MODE || mode == BOTH) {
            Serial.print(message);
        } 
        if ((mode == WIFI || mode == BOTH) && webSerialInitialized) {
            WebSerial.print(message);
        }
    }

    void log(const char* message) {
        log(String(message));
    }

    void log(int value) {
        log(String(value));
    }

    void log(float value) {
        log(String(value, 6));
    }

    void log(double value) {
        log(String(value, 6));
    }

    void log(bool value) {
        log(value ? "true" : "false");
    }

    void log(uint32_t value) {
        log(String(value));
    }

    void logln(const String& message) {
        log(message + "\n");
    }

    void logln(const char* message) {
        logln(String(message));
    }

    void logln(int value) {
        logln(String(value));
    }

    void logln(float value) {
        logln(String(value, 6));
    }

    void logln(double value) {
        logln(String(value, 6));
    }

    void logln(bool value) {
        logln(value ? "true" : "false");
    }

    void logln(uint32_t value) {
        logln(String(value));
    }

private:
    LogMode mode = SERIAL_MODE;
    bool webSerialInitialized = false;
};

extern UniLog logger;

#endif // LOGGER_H
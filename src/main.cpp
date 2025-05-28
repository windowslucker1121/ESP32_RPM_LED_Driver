#include <Arduino.h>
#include <WiFi.h>
#include "DevSensor.h"
#include "LedDriver.h"
#include "LeftToRightStyle.h"
#include "EqualFromBothSidesStyle.h"
#include <ESPAsyncWebServer.h>
#include "WebSerial.h"
#include "UniLog.h"
#include "WebServer.h"
#include "PWMGenerator.h"
#include "PWMReader.h"
#include "Settings.h"
#include "LedDisplayStyleRegistry.h"
#include <FastLED.h>
#include <nvs_flash.h>

bool connectToWifi();
Settings* settings = nullptr;
ISensor* sensor = new DevSensor();
LedDriver* ledDriver;
WebServer webServer(80, settings);
PWMGenerator* pwmGenerator;
bool isEnabled = true;

bool connectToWifi() {
    int wifiFailedCounter = 0;
    while (WiFi.waitForConnectResult(5000) != WL_CONNECTED && wifiFailedCounter < 10) {
        delay(1000);
        Serial.print(".");
        wifiFailedCounter++;
        if (wifiFailedCounter > 10) {
            Serial.println("Failed to connect to WiFi");
            break;
        }
    }
    if (wifiFailedCounter >= 10) {
        Serial.println("Failed to connect to WiFi");
        return false;
    }
    return true;
}

void setup() {

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    Serial.begin(115200);
    Serial.println("Starting...");

    const char* ssid = "yourSSID";
    const char* password = "yourPASSWSORD";
    

#if defined(USE_AP_MODE)
        const char* apSSID = "PWM_Reader_AP";
        const char* apPassword = "pwmreaderap";
        WiFi.softAP(apSSID, apPassword);
        Serial.println("Access Point started");
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
#elif defined(USE_STA_MODE)
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        int wifiFailedCounter = 0;
        Serial.print("Connecting to WiFi");
        if (!connectToWifi()) {
            Serial.println("\nFailed to connect to WiFi 10 times");
            Serial.println("Retrying one more time...");
            WiFi.disconnect();
            WiFi.begin(ssid, password);
            if (!connectToWifi()) {
                Serial.println("\nFailed to connect to WiFi again, staying in AP mode...");
                WiFi.softAP("PWM_Reader_AP", "12345678");
                Serial.println("Access Point started");
                Serial.print("AP IP: ");
                Serial.println(WiFi.softAPIP());
            }
        }

        if (WiFi.status() == WL_CONNECTED) {
            logger.logln("Connected to WiFi");
            logger.logln("IP: " + WiFi.localIP().toString());
        }
#endif

    // Settings::Erase(); // Uncomment this line to reset settings
    logger.logln("Loading settings...");
    settings = new Settings();
    settings->LoadSettings();

    isEnabled = settings->getIsEnabled();

    ledDriver = new LedDriver(LED_COUNT, settings);
    sensor = new PWMReader(settings);
    pwmGenerator = new PWMGenerator(settings);

    webServer.setup();
    logger.begin(UniLog::BOTH, &webServer.getServer());

    if (WiFi.status() == WL_CONNECTED) {
        logger.logln("Connected with IP: " + WiFi.localIP().toString());
    } else {
        logger.logln("Access Point IP: " + WiFi.softAPIP().toString());
    }

    sensor->setup(); 
    ledDriver->Setup();
    pwmGenerator->setup();

    sensor->SetInterval(50);
    ledDriver->SetInterval(50);
    pwmGenerator->SetInterval(200);

    // logger.isEnabled = false;
}

void loop() {
    webServer.loop();
    sensor->readValue(false);
    
    if (!isEnabled) {
        logger.log(".");
        return;
    }
    ledDriver->loop(false);
    pwmGenerator->loop(false);
}

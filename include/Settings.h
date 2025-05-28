#ifndef SETTINGS_H
#define SETTINGS_H

#include <memory>
#include <string>
#include "ILedDisplayStyle.h"
#include "LedDisplayStyleRegistry.h"
#include "UniLog.h"
#include <Preferences.h>
#include <nvs_flash.h>

class Settings {
public:
    Settings() :
        minReaderFrequency(5),
        maxReaderFrequency(100),
        minGeneratorFrequency(10),
        maxGeneratorFrequency(100),
        roundTimeGenerator(6000),
        currentAnalogValue(0),
        currentMappedValue(0),
        isEnabled(true),
        displayStyle(LedDisplayStyleRegistry::getInstance().createStyle("LeftToRight"))
    {
        logger.logln("Settings instance created at " + String(millis()));
    }

    ~Settings() = default;

    int getReaderMinFrequency() const { return minReaderFrequency; }
    void setReaderMinFrequency(int minFreq) { 
        minReaderFrequency = minFreq;
        logger.logln("Reader min frequency ("+String(minFreq)+ ") set to " + String(minReaderFrequency));
    }

    int getReaderMaxFrequency() const { return maxReaderFrequency; }
    void setReaderMaxFrequency(int maxFreq) { 
        maxReaderFrequency = maxFreq;
        logger.logln("Reader max frequency ("+String(maxFreq)+ ") set to " + String(maxReaderFrequency));
     }

    ILedDisplayStyle* getDisplayStyle() const { return displayStyle.get(); }
    String getDisplayStyleName() const { 
        return displayStyle ? displayStyle->getName() : "Unknown"; 
    }

    void setStyle(const String& newStyle) {
        logger.logln("Setting display style to: " + newStyle);
        displayStyle = LedDisplayStyleRegistry::getInstance().createStyle(newStyle.c_str());
        logger.logln("Display style set to " + newStyle);
    }

    int getCurrentMappedValue() const { return currentMappedValue; }
    void setCurrentMappedValue(int value) { currentMappedValue = value; }
    
    int getGeneratorMinFrequency() const { return minGeneratorFrequency; } 
    void setGeneratorMinFrequency(int minFreq) { minGeneratorFrequency = minFreq; }

    int getGeneratorMaxFrequency() const { return maxGeneratorFrequency; }
    void setGeneratorMaxFrequency(int maxFreq) { maxGeneratorFrequency = maxFreq; }

    int getRoundTimeGenerator() const { return roundTimeGenerator; }
    void setRoundTimeGenerator(int roundTime) { roundTimeGenerator = roundTime; }

    int getCurrentAnalogValue() const { return currentAnalogValue; }
    void setCurrentAnalogValue(int value) { currentAnalogValue = value; }

    int getLedBrightness() const { return ledBrightness; }
    void setLedBrightness(int brightness) {
        ledBrightness = brightness;
        logger.logln("LED brightness set to " + String(ledBrightness));
    }

    bool getIsEnabled() const { return isEnabled; }
    void setIsEnabled(bool enabled) {
        isEnabled = enabled;
        logger.logln("Settings enabled state set to " + String(enabled));
    }

    int getBootupAnimationTime() const { return bootupAnimationTime; }
    void setBootupAnimationTime(int time) {
        bootupAnimationTime = time;
        logger.logln("Bootup animation time set to " + String(bootupAnimationTime));
    }

    void SaveSettings() {
        Preferences preferences;
        if (preferences.begin("settings", false)) {
            size_t result = preferences.putInt("minReaderFreq", minReaderFrequency);
            logger.logln("Saving minReaderFreq: " + String(minReaderFrequency));
            if (result == 0) {
                logger.logln("Warning: Failed to save minReaderFreq");
            }
            
            result = preferences.putInt("maxReaderFreq", maxReaderFrequency);
            logger.logln("Saving maxReaderFreq: " + String(maxReaderFrequency));
            if (result == 0) {
                logger.logln("Warning: Failed to save maxReaderFreq");
            }
            

            result = preferences.putString("style", displayStyle->getName().c_str());
            logger.logln("Saving display style: " + displayStyle->getName());
            if (result == 0) {
                logger.logln("Warning: Failed to save display style");
            }
            
            
            result = preferences.putBool("isEnabled", isEnabled);
            logger.logln("Saving enabled state: " + String(isEnabled));
            if (result == 0) {
                logger.logln("Warning: Failed to save enabled state");
            }
            
            result = preferences.putInt("ledBrightness", ledBrightness);
            logger.logln("Saving ledBrightness: " + String(ledBrightness));
            if (result == 0) {
                logger.logln("Warning: Failed to save ledBrightness");
            }
            result = preferences.putInt("bootAnimTime", bootupAnimationTime);
            logger.logln("Saving bootupAnimationTime: " + String(bootupAnimationTime));
            if (result == 0) {
                logger.logln("Warning: Failed to save bootupAnimationTime");
            }

            preferences.end();
            logger.logln("Settings saved (See previous logs if something failed).");
        } else {
            logger.logln("Error: Failed to open preferences for saving.");
        }
    }
    

    static void Erase() {
        logger.logln("Erasing NVS flash storage...");
        nvs_flash_erase();
        nvs_flash_init();
        while (true)
        {
            
            logger.logln("NVS flash storage erased. Please uncomment in main.cpp setup() to reinitialize settings.");
            delay(5000);
        }
        
    }

    void LoadSettings() {
        Preferences preferences;
        logger.logln("Attempting to open preferences namespace...");
        if (!preferences.begin("settings", false)) {
            logger.logln("Failed to open preferences namespace");
            SaveSettings();
            return;
        }
        logger.logln("Preferences namespace opened successfully.");
    
        logger.logln("Loading settings from preferences...");
        if (preferences.isKey("minReaderFreq")) {
            logger.logln("Loading minReaderFreq from preferences: " + String(preferences.getInt("minReaderFreq")));
            minReaderFrequency = preferences.getInt("minReaderFreq", minReaderFrequency);
        } else {
            logger.logln("minReaderFreq not found in preferences. Using default: " + String(minReaderFrequency));
        }
    
        if (preferences.isKey("maxReaderFreq")) {
            logger.logln("Loading maxReaderFreq from preferences: " + String(preferences.getInt("maxReaderFreq")));
            maxReaderFrequency = preferences.getInt("maxReaderFreq", maxReaderFrequency);
        } else {
            logger.logln("maxReaderFreq not found in preferences. Using default: " + String(maxReaderFrequency));
        }
    
        if (preferences.isKey("style")) {
            String style = preferences.getString("style", "LeftToRight");
            logger.logln("Loading style from preferences: " + style);
            displayStyle = LedDisplayStyleRegistry::getInstance().createStyle(style.c_str());
        } else {
            logger.logln("Style not found in preferences. Using default: LeftToRight");
            displayStyle = LedDisplayStyleRegistry::getInstance().createStyle("LeftToRight");
        }
    
        if (preferences.isKey("isEnabled")) {
            isEnabled = preferences.getBool("isEnabled", true);
            logger.logln("Loading isEnabled from preferences: " + String(isEnabled));
        } else {
            logger.logln("isEnabled not found in preferences. Using default: true");
            isEnabled = true;
        }

        if (preferences.isKey("ledBrightness")) {
            ledBrightness = preferences.getInt("ledBrightness", 255);
            logger.logln("Loading ledBrightness from preferences: " + String(ledBrightness));
        } else {
            logger.logln("ledBrightness not found in preferences. Using default: 255");
            ledBrightness = 255;
        }

        if (preferences.isKey("bootAnimTime")) {
            bootupAnimationTime = preferences.getInt("bootAnimTime", 3000);
            logger.logln("Loading bootupAnimationTime from preferences: " + String(bootupAnimationTime));
        } else {
            logger.logln("bootupAnimationTime not found in preferences. Using default: 3000");
            bootupAnimationTime = 3000;
        }
        
        logger.logln("Settings loaded from preferences.");
        preferences.end();
    }
    
private:
    int minReaderFrequency;
    int maxReaderFrequency;
    int minGeneratorFrequency;
    int maxGeneratorFrequency;
    int roundTimeGenerator;
    int currentAnalogValue;
    int currentMappedValue;
    int ledBrightness;
    bool isEnabled = true;
    int bootupAnimationTime;

    std::unique_ptr<ILedDisplayStyle> displayStyle;
};

#endif // SETTINGS_H

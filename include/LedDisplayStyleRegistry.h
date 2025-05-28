#ifndef LED_DISPLAY_STYLE_REGISTRY_H
#define LED_DISPLAY_STYLE_REGISTRY_H

#include <vector>
#include <stdint.h>
#include <map>
#include <memory>
#include <string>
#include <functional>
#include "ILedDisplayStyle.h"
#include "LeftToRightStyle.h"
#include "EqualFromBothSidesStyle.h"
#include "FlashingLeftToRightStyle.h"
#include "RightToLeftStyle.h"
#include "CustomLedStyle.h"
#include <Preferences.h>
#include "UniLog.h"


class LedDisplayStyleRegistry {
    public:
        using StyleFactory = std::function<std::unique_ptr<ILedDisplayStyle>()>;
    
        static LedDisplayStyleRegistry& getInstance() {
            static LedDisplayStyleRegistry instance;
            return instance;
        }

        LedDisplayStyleRegistry() {
            registerStyle("EqualFromBothSides", []() { return std::unique_ptr<ILedDisplayStyle>(new EqualFromBothSidesStyle()); });
            registerStyle("LeftToRight", []() { return std::unique_ptr<ILedDisplayStyle>(new LeftToRightStyle()); });
            registerStyle("FlashingLeftToRight", []() { return std::unique_ptr<ILedDisplayStyle>(new FlashingLeftToRightStyle()); });
            registerStyle("RightToLeft", []() { return std::unique_ptr<ILedDisplayStyle>(new RightToLeftStyle()); });
            loadCustomStyles();
            
        }

        void registerStyle(const std::string& name, StyleFactory factory) {
            logger.logln("Registering style in factory: " + String(name.c_str()));
            styleFactories[name] = factory;
            logger.logln("style colorSchema: " + String(factory().get()->getColorSchema().ToString()));
            logger.logln("Registered style: " + String(name.c_str()) + " with factory at address: " + String(reinterpret_cast<uintptr_t>(&styleFactories[name])));
        }
    
        std::unique_ptr<ILedDisplayStyle> createStyle(const std::string& name) {
            
            std::string cleanName(name);
            cleanName.erase(std::remove(cleanName.begin(), cleanName.end(), '\0'), cleanName.end());
            
            logger.logln("Searching for: [" + String(cleanName.c_str()) + "]");
        
            auto it = styleFactories.find(cleanName);
            if (it != styleFactories.end()) {
                return it->second();
            }
        
            auto lowerName = toLower(cleanName);
            for (const auto& pair : styleFactories) {
                if (toLower(pair.first) == lowerName) {
                    logger.logln("Found case-insensitive match for: " + String(pair.first.c_str()));
                    return pair.second();
                }
            }
            
            logger.logln("Style not found after exhaustive search");
            return nullptr;
        }
        
        std::string toLower(const std::string& str) {
            std::string result(str);
            std::transform(result.begin(), result.end(), result.begin(),
                [](unsigned char c){ return std::tolower(c); });
            return result;
        }
    
        std::vector<std::string> getRegisteredStyleNames() const {
            std::vector<std::string> names;
            for (const auto& pair : styleFactories) {
                names.push_back(pair.first);
            }
            return names;
        }

        bool saveCustomStyle(const String& name, const String& jsonConfig) {
            Preferences preferences;
            if (preferences.begin("ledStyles", false)) {
                preferences.putString(name.c_str(), jsonConfig);
                
                std::vector<String> customNames = getCustomStyleNames();
                if (std::find(customNames.begin(), customNames.end(), name) == customNames.end()) {
                    size_t count = preferences.getUInt("count", 0);
                    preferences.putString(("style" + std::to_string(count) + "_name").c_str(), name.c_str());
                    preferences.putUInt("count", count + 1);
                }
                
                preferences.end();
                
                registerCustomStyle(name, jsonConfig);
                return true;
            }
            return false;
        }
        
        bool deleteCustomStyle(const String& name) {
            Preferences preferences;
            bool result = false;
            
            if (preferences.begin("ledStyles", false)) {
                result = preferences.remove(name.c_str());
                
                std::vector<String> names;
                size_t count = preferences.getUInt("count", 0);
                size_t newCount = 0;
                
                for (size_t i = 0; i < count; i++) {
                    String key = "style" + String(i) + "_name";
                    String styleName = preferences.getString(key.c_str(), "");
                    
                    if (styleName.length() > 0 && styleName != name) {
                        preferences.putString(("style" + String(newCount) + "_name").c_str(), styleName);
                        newCount++;
                    }
                    
                    preferences.remove(key.c_str());
                }
                
                preferences.putUInt("count", newCount);
                preferences.end();
                
                styleFactories.erase(name.c_str());
            }
            
            return result;
        }
        
        std::vector<String> getCustomStyleNames() {
            std::vector<String> names;
            Preferences preferences;
            
            if (preferences.begin("ledStyles", true)) {
                size_t count = preferences.getUInt("count", 0);
                
                for (size_t i = 0; i < count; i++) {
                    String key = "style" + String(i) + "_name";
                    String styleName = preferences.getString(key.c_str(), "");
                    
                    if (styleName.length() > 0) {
                        names.push_back(styleName);
                    }
                }
                
                preferences.end();
            }
            else {
                logger.logln("It seems like no custom styles are registered - cant return any custom style names.");
            }
            return names;
        }
    
    private:
        void registerCustomStyle(const String& name, const String& jsonConfig) {
            logger.logln("Registering custom style (registerCustomStyle one) " + name);
            registerStyle(name.c_str(), [name, jsonConfig]() {
                logger.logln("Registering custom style (RegisterCustomStyle two) " + name);
                return std::unique_ptr<ILedDisplayStyle>(CustomLedStyle::fromJson(jsonConfig));
            });
        }
        
        void loadCustomStyles() {
            Preferences preferences;
            if (preferences.begin("ledStyles", true)) {
                logger.logln("Loading custom styles from preferences...");
                size_t count = preferences.getUInt("count", 0);
                logger.logln("Found " + String(count) + " custom styles.");
                for (size_t i = 0; i < count; i++) {
                    String key = "style" + String(i) + "_name";
                    String styleName = preferences.getString(key.c_str(), "");
                    logger.logln("Loading custom style: " + styleName);
                    if (styleName.length() > 0) {
                        String jsonConfig = preferences.getString(styleName.c_str(), "");
                        logger.logln("JSON Config for " + styleName + ": " + jsonConfig);
                        if (jsonConfig.length() > 0) {
                            logger.logln("Registering custom style (loadCustomSTyle function): " + styleName);
                            registerCustomStyle(styleName, jsonConfig);
                        }
                    }
                }
                
                preferences.end();
            }
            else
            {
                logger.logln("It seems like no custom styles are saved - cant load any custom styles.");
            }
        }
    
    private:
        std::map<std::string, StyleFactory> styleFactories;
    };

#endif // LED_DISPLAY_STYLE_REGISTRY_H
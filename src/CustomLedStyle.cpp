#include "CustomLedStyle.h"
#include "UniLog.h"

CustomLedStyle::CustomLedStyle(const String& name, StylePattern pattern, 
                            const ColorSchema& colors, bool enableFlashing, 
                            int flashInterval)
    : name(name), pattern(pattern), colorSchema(colors),
      enableFlashing(enableFlashing), flashInterval(flashInterval) {
}

void CustomLedStyle::getLedStates(int value, std::vector<bool>& ledStates) {
    const int numLeds = ledStates.size();
    int numLedsToLight = (value * numLeds) / 100;
    
    if (value % 10 >= 5 && numLedsToLight < numLeds) {
        numLedsToLight++;
    }
    
    if (enableFlashing && value >= 100 - colorSchema.highPercent) {
        unsigned long currentTime = millis();
        if (currentTime - lastFlashToggle >= flashInterval) {
            flashState = !flashState;
            lastFlashToggle = currentTime;
        }
    } else {
        flashState = true;
    }
    
    for (int i = 0; i < numLeds; ++i) {
        bool shouldLight = false;
        
        switch (pattern) {
            case LEFT_TO_RIGHT:
                shouldLight = i < numLedsToLight;
                break;
                
            case RIGHT_TO_LEFT:
                shouldLight = i >= numLeds - numLedsToLight;
                break;
                
            case CENTER_TO_EDGES: {
                int middle = numLeds / 2;
                int halfLedsToLight = numLedsToLight / 2;
                shouldLight = (i >= middle - halfLedsToLight) && (i <= middle + halfLedsToLight);
                break;
            }
            
            case EDGES_TO_CENTER: {
                int halfLedsToLight = numLedsToLight / 2;
                shouldLight = (i < halfLedsToLight) || (i >= numLeds - halfLedsToLight);
                break;
            }
        }
        
        ledStates[i] = shouldLight && (flashState || value < 100 - colorSchema.highPercent);
    }
}

String CustomLedStyle::toJson() const {
    StaticJsonDocument<512> doc;
    
    doc["name"] = name;
    doc["pattern"] = (int)pattern;
    
    JsonObject colors = doc.createNestedObject("colors");
    colors["lowR"] = colorSchema.lowColor.r;
    colors["lowG"] = colorSchema.lowColor.g;
    colors["lowB"] = colorSchema.lowColor.b;
    colors["midR"] = colorSchema.midColor.r;
    colors["midG"] = colorSchema.midColor.g;
    colors["midB"] = colorSchema.midColor.b;
    colors["highR"] = colorSchema.highColor.r;
    colors["highG"] = colorSchema.highColor.g;
    colors["highB"] = colorSchema.highColor.b;
    colors["lowPercent"] = colorSchema.lowPercent;
    colors["highPercent"] = colorSchema.highPercent;
    
    doc["enableFlashing"] = enableFlashing;
    doc["flashInterval"] = flashInterval;
    
    String output;
    serializeJson(doc, output);
    return output;
}

CustomLedStyle* CustomLedStyle::fromJson(const String& jsonString) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    logger.logln("Deserializing JSON: " + jsonString);
    if (error) {
        logger.logln("Failed to deserialize JSON: " + String(error.c_str()));
        return nullptr;
    }
    
    String name = doc["name"].as<String>();
    StylePattern pattern = (StylePattern)doc["pattern"].as<int>();
    
    ColorSchema colorSchema;
    JsonObject colors = doc["colors"];
    colorSchema.lowColor = CRGB(colors["lowR"], colors["lowG"], colors["lowB"]);
    colorSchema.midColor = CRGB(colors["midR"], colors["midG"], colors["midB"]);
    colorSchema.highColor = CRGB(colors["highR"], colors["highG"], colors["highB"]);
    colorSchema.lowPercent = colors["lowPercent"];
    colorSchema.highPercent = colors["highPercent"];
    
    bool enableFlashing = doc["enableFlashing"];
    int flashInterval = doc["flashInterval"];
    logger.logln("Creating CustomLedStyle from JSON: " + name);
    return new CustomLedStyle(name, pattern, colorSchema, enableFlashing, flashInterval);
}

#ifndef ILED_DISPLAY_STYLE_H
#define ILED_DISPLAY_STYLE_H
#include <vector>
#include <Arduino.h>
#include "ColorSchema.h"
class ILedDisplayStyle {
    public:
        virtual ~ILedDisplayStyle() {}
        
        virtual void getLedStates(int value, std::vector<bool>& ledStates) = 0;
        virtual String getName() = 0;
        virtual ColorSchema getColorSchema() {
            return colorSchema;
        }

    protected:
        ColorSchema colorSchema = ColorSchema();
    };


#endif // ILED_DISPLAY_STYLE_H
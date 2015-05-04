#ifndef SCOUT_SETTINGS_H_
#define SCOUT_SETTINGS_H_

#include <Arduino.h>

namespace pinoccio {
    class Settings {
        public:
            void setup();

            uint32_t getHwSerial();
            uint16_t getHwFamily();
            uint8_t getHwVersion();
            uint8_t getEEPROMVersion();

            uint8_t* getSecurityKey();
            uint8_t getRadioPower();
            uint8_t getRadioDataRate();
            uint8_t getRadioChannel();
            uint16_t getAddress();
            uint16_t getPanId();

            int8_t getTemperatureOffset(void);
            void setTemperatureOffset(int8_t offset);

            void setOTAFlag();

        protected:
            int8_t tempOffset;
    };
}
#endif

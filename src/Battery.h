#ifndef SCOUT_BATTERY_H_
#define SCOUT_BATTERY_H_

namespace pinoccio {
    class Battery {
        public:
            void setup();

            bool isCharging();
            bool isAlarmTriggered();
            bool isConnected();

            int getPercentage();
            int getVoltage();
    };
}

#endif

#include <Battery.h>

#include "peripherals/halFuelGauge.h"

using namespace pinoccio;

void Battery::setup() {
    pinMode(CHG_STATUS, INPUT_PULLUP);
    pinMode(BATT_ALERT, INPUT_PULLUP);

    HAL_FuelGaugeConfig(20);   // Configure the MAX17048G's alert percentage to 20%
}

bool Battery::isCharging() {
    return (digitalRead(CHG_STATUS) == LOW);
}

int Battery::getPercentage() {
    return constrain(HAL_FuelGaugePercent(), 0, 100);
}

int Battery::getVoltage() {
    return HAL_FuelGaugeVoltage();
}

bool Battery::isAlarmTriggered() {
    return (digitalRead(BATT_ALERT) == LOW);
}

bool Battery::isConnected() {
    bool start = digitalRead(CHG_STATUS);
    bool state = start;
    bool changed = false;

    for (int i = 0; i < 40; i++) {
        if ((state = digitalRead(CHG_STATUS)) != start) {
            changed = true;
        } else if (changed) {
            return false;
        }
        delay(1);
    }
    return true;
}


#include <Arduino.h>
#include <Backpack.h>

using namespace pinoccio;

void Backpack::setup() {
    enableVcc();
    delay(100);
    disableVcc();
}

void Backpack::enableVcc() {
    vccEnabled = true;
    digitalWrite(VCC_ENABLE, HIGH);
}

void Backpack::disableVcc() {
    vccEnabled = false;
    digitalWrite(VCC_ENABLE, LOW);
}

bool Backpack::isVccEnabled() {
    return vccEnabled;
}


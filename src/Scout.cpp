/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <Wire.h>

#include "Scout.h"
#include "peripherals/halTemperature.h"

using namespace pinoccio;

Scout scout;

Scout::Scout() {
    // Read last reset
    lastResetCause = GPIOR0;
    isFactoryResetReady = false;
}

void Scout::setup(const char *name) {
    this->name = name;
    settings.setup();

    Wire.begin();
    Serial.begin(115200);

    digitalWrite(SS, HIGH);
    pinMode(SS, OUTPUT);
    pinMode(VCC_ENABLE, OUTPUT);

    battery.setup();
    backpack.setup();
    sleep.setup();

    mesh.setup(settings);

    commands.setup();
    modules.setup(this);

    led.turnOff();
}

void Scout::loop() {
    mesh.loop();
    sleep.loop();

    modules.loop();
}

void Scout::reboot() {
    cli();
    wdt_enable(WDTO_15MS);
    while (1);
}

const char *Scout::getLastResetCause() {
    switch (lastResetCause) {
        case 1:
            return PSTR("Power-on");
        case 2:
            return PSTR("External");
        case 4:
            return PSTR("Brown-out");
        case 8:
            return PSTR("Watchdog");
        case 16:
            return PSTR("JTAG");
        default:
            return PSTR("Unknown Cause Reset");
    }
}

int8_t Scout::getTemperature() {
    // TODO external Aref check
    return HAL_MeasureTemperature() + settings.getTemperatureOffset();
}


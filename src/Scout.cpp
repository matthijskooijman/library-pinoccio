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

#include <Scout.h>
#include "peripherals/halTemperature.h"

using namespace pinoccio;

Scout scout;

Scout::Scout() {
  lastResetCause = GPIOR0;
  isFactoryResetReady = false;
}

void Scout::setup() {

  settings.setup();

  Wire.begin();
  Serial.begin(115200);

  digitalWrite(SS, HIGH);
  pinMode(SS, OUTPUT);
  pinMode(VCC_ENABLE, OUTPUT);

  mesh.setup(settings);
  battery.setup();
  backpack.setup();
  sleep.setup();

  led.turnOff();
}

void Scout::loop() {
  mesh.loop();
  sleep.loop();
}

void Scout::reboot() {
  cli();
  wdt_enable(WDTO_15MS);
  while(1);
}

const char* Scout::getLastResetCause() {
  switch (lastResetCause) {
      case 1:
        return PSTR("Power-on");
        break;
      case 2:
        return PSTR("External");
        break;
      case 4:
        return PSTR("Brown-out");
        break;
      case 8:
        return PSTR("Watchdog");
        break;
      case 16:
        return PSTR("JTAG");
        break;
      default:
        return PSTR("Unknown Cause Reset");
  }
}

int8_t Scout::getTemperature() {
  // TODO external Aref check
  return HAL_MeasureTemperature() + settings.getTemperatureOffset();
}


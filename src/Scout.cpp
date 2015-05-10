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

void Scout::report(const char *type, cn_cbor *data) {
  // Wrap the data into a report CBOR array:
  // 0:<type> 1:<timestamp> 2:data
  cn_cbor *wrapper;
  cn_cbor *typeCb;
  cn_cbor *timeCb;

  cn_cbor_errback err;
  wrapper = cn_cbor_array_create(NULL);
  typeCb = cn_cbor_string_create(type, NULL);
  timeCb = cn_cbor_int_create(millis(), NULL);

  cn_cbor_array_append(wrapper, typeCb, NULL);
  cn_cbor_array_append(wrapper, timeCb, NULL);
  cn_cbor_array_append(wrapper, data, NULL);

  mesh.broadcast(REPORT_ENDPOINT, REPORT_ENDPOINT, wrapper);

  // Clean up
  cn_cbor_free(wrapper);
}

bool Scout::addCommand(const char *name, uint8_t (*handler)(const cn_cbor *args)) {
  commands.addCommand(name, handler);
}

void Scout::sendCommand(uint16_t address, const char *name, cn_cbor *args) {
  commands.sendCommand(address, name, args);
}

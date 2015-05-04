/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include <Wire.h>
#include <Scout.h>
#include "SleepHandler.h"
#include <math.h>
#include <avr/pgmspace.h>
#include "peripherals/halTemperature.h"

using namespace pinoccio;

Scout scout;

Scout::Scout() {
  isFactoryResetReady = false;
  sleepPending = false;
}

Scout::~Scout() { }

void Scout::setup() {
  settings.setup();

  digitalWrite(SS, HIGH);
  pinMode(SS, OUTPUT);
  pinMode(VCC_ENABLE, OUTPUT);

  Serial.begin(115200);

  mesh.setup(settings);
  battery.setup();
  backpack.setup();

  SleepHandler::setup();

  led.turnOff();
  Wire.begin();
}

void Scout::loop() {

  mesh.loop();

  bool canSleep = true;
  // TODO: Let other loop functions return some "cansleep" status as well


  if (sleepPending) {
    canSleep = canSleep && !NWK_Busy();

    // if remaining <= 0, we won't actually sleep anymore, but still
    // call doSleep to run the callback and clean up
    if (SleepHandler::scheduledTicksLeft() == 0)
      doSleep(true);
    else if (canSleep)
      doSleep(false);
  }
}

bool Scout::factoryReset() {
  if (!isFactoryResetReady) {
    isFactoryResetReady = true;
    return false;
  } else {
    return true;
  }
}

void Scout::reboot() {
  cli();
  wdt_enable(WDTO_15MS);
  while(1);
}

void Scout::scheduleSleep(uint32_t ms, const char *func) {
  // TODO: how are we going to do the callback?
  if (ms) {
    SleepHandler::scheduleSleep(ms);
    sleepPending = true;
  } else {
    sleepPending = false;
  }
  sleepMs = ms;
}

void Scout::doSleep(bool pastEnd) {
  // Copy the pointer, so the post command can set a new sleep
  // timeout again.
  sleepPending = false;

  if (!pastEnd) {
    NWK_SleepReq();

    // TODO: suspend more stuff? Wait for UART byte completion?

    SleepHandler::doSleep(true);
    NWK_WakeupReq();
  }

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


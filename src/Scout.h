/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_SCOUT_H_
#define LIB_PINOCCIO_SCOUT_H_

#include <Backpack.h>
#include <Battery.h>
#include <Commands.h>
#include <Mesh.h>
#include <Modules.h>
#include <Settings.h>
#include <Sleep.h>
#include <Wire.h>

#include <cbor.h>
#include "cn-cbor/cn-cbor.h"

#include "peripherals/halRgbLed.h"

#define REPORT_ENDPOINT 1

namespace pinoccio {

class Scout {
  public:
    Scout();

    Settings settings;
    Battery battery;
    Backpack backpack;
    HalRgbLed led;
    Mesh mesh;
    Sleep sleep;
    Modules modules;
    Commands commands;

    void setup();
    void loop();

    bool factoryReset();
    void reboot();

    const char* getLastResetCause();
    int8_t getTemperature();

    void report(const char *type, cn_cbor *data);

    bool addCommand(const char *name, uint8_t (*handler)(const cn_cbor *args));
    void sendCommand(uint16_t address, const char *name, cn_cbor *args);

  protected:
    bool isFactoryResetReady;
    uint8_t lastResetCause;

};
}

extern pinoccio::Scout scout;

#endif

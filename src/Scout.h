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
#include <Settings.h>
#include <Mesh.h>
#include <Wire.h>

#include "peripherals/halRgbLed.h"

namespace pinoccio {

class Scout {

  public:
    Scout();
    ~Scout();

    void setup();
    void loop();

    bool factoryReset();
    void reboot();

    const char* getLastResetCause();
    int8_t getTemperature();

    Settings settings;
    Battery battery;
    Backpack backpack;
    HalRgbLed led;
    Mesh mesh;

    // Schedule a sleep that lasts until now + ms. The optional bitlash
    // command is executed after the sleep. A previous sleep can be
    // canceled by passing 0, NULL. The command passed in will be
    // copied, so it does not have to remain valid.
    void scheduleSleep(uint32_t ms, const char *cmd);

  protected:

    void doSleep(bool pastEnd);

    bool isStateSaved;
    bool isFactoryResetReady;

    bool sleepPending;
    // The original sleep time, used to pass to the callback and to
    // re-sleep. The actual sleep time for the next sleep is stored by
    // SleepHandler instead.
    uint32_t sleepMs;

    uint8_t lastResetCause;
};

}

extern pinoccio::Scout scout;

#endif

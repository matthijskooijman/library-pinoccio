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
#include <Mesh.h>
#include <Settings.h>
#include <Sleep.h>
#include <Wire.h>

#include "peripherals/halRgbLed.h"

namespace pinoccio {

class Scout {

  public:
    Scout();

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
    Sleep sleep;

  protected:

    bool isFactoryResetReady;

    uint8_t lastResetCause;
};

}

extern pinoccio::Scout scout;

#endif

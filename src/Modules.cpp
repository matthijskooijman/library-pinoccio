/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include "Scout.h"
#include "Modules.h"
#include "modules/Module.h"

using namespace pinoccio;

Module* Modules::_modules = NULL;

Modules::Modules() {
}

void Modules::setup(Scout *scout) {
  Module *module = modules();
  while (module) {
    module->setup(scout);
    module = module->next();
  }
}

void Modules::loop() {
  Module *module = modules();
  while (module) {
    module->loop();
    module = module->next();
  }
}


/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_MODULE_H_
#define LIB_PINOCCIO_MODULE_H_

#include <Arduino.h>
#include <Scout.h>
#include <Modules.h>

namespace pinoccio {
    class Module {
    public:
        Module() {
            // Prepend ourselves to the list of modules
            this->_next = Modules::_modules;
            Modules::_modules = this;
        }

        /**
         * Called on setup
         */
        virtual void setup(Scout *scout) = 0;

        /**
         * Called on every loop
         */
        virtual void loop() = 0;

        /**
         * Next element in the list of modules
         */
        Module *next() const { return _next; }

    private:
        Module *_next;

        // Let Modules update _enabled
        friend class Modules;
    };
} // namespace pinoccio
#endif

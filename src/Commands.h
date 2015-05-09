#ifndef SCOUT_COMMANDS_H_
#define SCOUT_COMMANDS_H_

#include <Arduino.h>
#include <cbor.h>
#include "cn-cbor/cn-cbor.h"

#define COMMAND_ENDPOINT 2

namespace pinoccio {
    class Commands {
        public:
            void setup();

            bool addCommand(const char* name, void (*handler)(const cn_cbor *args));
            void sendCommand(uint16_t address, const char *command, cn_cbor *args);
    };
}

#endif

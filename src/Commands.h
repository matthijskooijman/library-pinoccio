#ifndef SCOUT_COMMANDS_H_
#define SCOUT_COMMANDS_H_

#include <Arduino.h>
#include <cbor.h>
#include "cn-cbor/cn-cbor.h"
#include "peripherals/halFuelGauge.h"

#define REPORT_ENDPOINT 1
#define COMMAND_ENDPOINT 2
#define REPLY_ENDPOINT 3

namespace pinoccio {
    class Commands {
    public:
        void setup();

        bool add(const char *name, cn_cbor *(*handler)(cn_cbor *args));

        void send(uint16_t address, const char *command, cn_cbor *args);

        void report(const char *type, cn_cbor *data);

        uint16_t query(uint16_t address, const char *command, cn_cbor *args,
                       void (*callback)(uint16_t id, cn_cbor *args));

        uint16_t raw(uint16_t address, const char *command, cn_cbor *args, uint16_t id,
                     void (*callback)(uint16_t id, cn_cbor *args));

    };
}

#endif

#include <Commands.h>
#include <Scout.h>
#include <avr/eeprom.h>

using namespace pinoccio;

#define MAX_COMMANDS 16
#define MAX_QUERIES 16

//
// Commands are [command, args, id?] if no id given, no reply is sent
// Replies are [id, args]
//

struct command_t {
    const char *name;

    cn_cbor *(*handler)(cn_cbor *args); // TODO: maybe add src address/endpoint to the callback?
};

struct query_t {
    uint16_t id;

    void (*callback)(uint16_t id, cn_cbor *args);
};

command_t commands[MAX_COMMANDS];
query_t queries[MAX_QUERIES];
uint8_t commandCount = 0;
uint8_t queryIdCounter = 0;
uint8_t queryCounter = 0;

bool handleCommand(uint16_t dstAddress, uint8_t dstEndpoint, cn_cbor *args);

bool handleReply(uint16_t srcAddress, uint8_t srcEndpoint, cn_cbor *args);

void reply(uint16_t address, uint8_t endpoint, uint16_t queryId, cn_cbor *args);

//
// Built-in commands
//

cn_cbor *reboot(cn_cbor *args);

cn_cbor *otaMode(cn_cbor *args);

cn_cbor *led(cn_cbor *args);

cn_cbor *fuel(cn_cbor *args);

cn_cbor *name(cn_cbor *args);


void Commands::setup() {
    // Add our default commands
    add("otamode", otaMode);
    add("reboot", reboot);
    add("led", led);
    add("fuel", fuel);
    add("name", name);

    // Start listening on the command endpoint and the reply endpoint
    scout.mesh.listen(COMMAND_ENDPOINT, handleCommand);
    scout.mesh.listen(REPLY_ENDPOINT, handleReply);
}

bool handleCommand(uint16_t dstAddress, uint8_t dstEndpoint, cn_cbor *data) {
    // We expect 'command' and 'args', and optionally 'id'
    cn_cbor *name = cn_cbor_index(data, 0);
    cn_cbor *payload = cn_cbor_index(data, 1);
    cn_cbor *id = (data->length == 3) ? cn_cbor_index(data, 2) : NULL;

    // Loop over commands to find it
    bool found = false;
    for (uint8_t idx = 0; idx < MAX_COMMANDS; idx++) {
        if (strncmp(name->v.str, commands[idx].name, name->length) == 0) {
            found = true;

            cn_cbor *ret = commands[idx].handler(payload);

            // Check if we need to respond
            if (id != NULL) {
                reply(dstAddress, dstEndpoint, id->v.uint, ret);
            }
            break;
        }
    }

    // TODO: if command not found, mention it to sender
    return found;
}

bool handleReply(uint16_t srcAddress, uint8_t srcEndpoint, cn_cbor *data) {
    // We expect 'id' and 'args'
    cn_cbor *id = cn_cbor_index(data, 0);
    cn_cbor *payload = cn_cbor_index(data, 1);

    // Loop over current queries to find it
    bool found = false;
    for (uint8_t idx = 0; idx < MAX_QUERIES; idx++) {
        if (queries[idx].id == id->v.uint) {
            found = true;
            queries[idx].callback(queries[idx].id, payload);
            queries[idx].id = NULL;
        }
    }

    // TODO: if query not found, mention it to sender
    return found;
}

void reply(uint16_t address, uint8_t endpoint, uint16_t queryId, cn_cbor *args) {
    // Build a reply structure wrapper in which we put 'id' and 'args'
    cn_cbor *wrapper = cn_cbor_array_create(NULL);

    cn_cbor_array_append(wrapper, cn_cbor_int_create(queryId, NULL), NULL);
    cn_cbor_array_append(wrapper, args, NULL);

    // Send it off
    scout.mesh.send(address, COMMAND_ENDPOINT, endpoint, wrapper);

    // We free our wrapper and thus also the original 'args' structure
    cn_cbor_free(wrapper);
}

bool Commands::add(const char *name, cn_cbor *(*handler)(cn_cbor *args)) {
    // Check our ceiling for commands
    if (commandCount == MAX_COMMANDS) {
        return false;
    }

    // Copy over the command name in our structure and remember the handler
    uint8_t idx = commandCount++;
    commands[idx].name = strdup(name);
    commands[idx].handler = handler;
    return true;
}

void Commands::send(uint16_t address, const char *command, cn_cbor *args) {
    // Send is nothing else than not caring about any response
    raw(address, command, args, NULL, NULL);
}

uint16_t Commands::query(uint16_t address, const char *command, cn_cbor *args,
                         void (*callback)(uint16_t id, cn_cbor *args)) {
    uint16_t queryId = ++queryIdCounter;
    if (queryId == NULL) {
        queryId = ++queryIdCounter;
    }
    return raw(address, command, args, queryId, callback);
}

uint16_t Commands::raw(uint16_t address, const char *command, cn_cbor *args, uint16_t queryId,
                       void (*callback)(uint16_t id, cn_cbor *args)) {
    cn_cbor *wrapper = cn_cbor_array_create(NULL);

    cn_cbor_array_append(wrapper, cn_cbor_string_create(command, NULL), NULL);
    cn_cbor_array_append(wrapper, args, NULL);

    if (queryId != NULL) {
        // Save the query callback
        uint8_t queryIdx = (queryCounter++ % MAX_QUERIES);
        queries[queryIdx].callback = callback;
        queries[queryIdx].id = queryId;
        cn_cbor_array_append(wrapper, cn_cbor_int_create(queryId, NULL), NULL);
    }

    // First check whether we are addressing ourselves
    if (address == 0 || address == scout.settings.getAddress()) {
        handleCommand(0, COMMAND_ENDPOINT, wrapper);
    } else {
        scout.mesh.send(address, REPLY_ENDPOINT, COMMAND_ENDPOINT, wrapper);
    }

    cn_cbor_free(wrapper);

    return queryId;
}

void Commands::report(const char *type, cn_cbor *data) {
    // Wrap the data into a report CBOR array:
    // 0:<type> 1:<timestamp> 2:data
    cn_cbor *wrapper;
    cn_cbor *typeCb;
    cn_cbor *timeCb;

    wrapper = cn_cbor_array_create(NULL);
    typeCb = cn_cbor_string_create(type, NULL);
    timeCb = cn_cbor_int_create(millis(), NULL);

    cn_cbor_array_append(wrapper, typeCb, NULL);
    cn_cbor_array_append(wrapper, timeCb, NULL);
    cn_cbor_array_append(wrapper, data, NULL);

    scout.mesh.broadcast(REPORT_ENDPOINT, REPORT_ENDPOINT, wrapper);

    // Clean up
    cn_cbor_free(wrapper);
}

//
// Default commands
//

cn_cbor *reboot(cn_cbor *args) {
    scout.reboot();
    return NULL;
}

cn_cbor *otaMode(cn_cbor *args) {
    scout.settings.setOTAFlag();
    scout.reboot();
    return NULL;
}

cn_cbor *led(cn_cbor *args) {
    if (args->length == 0) {
        // No args -> off
        scout.led.turnOff();
    } else {
        uint8_t red = (uint8_t) cn_cbor_mapget_string(args, "r")->v.uint;
        uint8_t green = (uint8_t) cn_cbor_mapget_string(args, "g")->v.uint;
        uint8_t blue = (uint8_t) cn_cbor_mapget_string(args, "b")->v.uint;
        const cn_cbor *blink = cn_cbor_mapget_string(args, "blink");

        // Set the led
        if (blink != NULL && blink->v.uint) {
            scout.led.blinkColor(red, green, blue);
        } else {
            scout.led.setColor(red, green, blue);
        }
    }
    return NULL;
}

cn_cbor *fuel(cn_cbor *args) {
    cn_cbor *data = cn_cbor_int_create(HAL_FuelGaugePercent(), NULL);
    return data;
}

cn_cbor *name(cn_cbor *args) {
    if (args->length == 0) {
        // return name
        const char *name;
        eeprom_read_block((void *) name, (void *) (SCOUT_EEPROM_BASE + 1), 10);
        cn_cbor *data = cn_cbor_string_create(name, NULL);
        return data;
    } else {
        // write name
        eeprom_write_block((const void *) args->v.str, (void *) (SCOUT_EEPROM_BASE + 1), 10);
    }
    return NULL;
}


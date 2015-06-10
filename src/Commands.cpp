#include <Commands.h>
#include <Scout.h>

using namespace pinoccio;

#define MAX_COMMANDS 16
#define MAX_QUERIES 16

//
//
//
//

struct command_t {
  const char* name;
  cn_cbor* (*handler)(const cn_cbor *args); // TODO: maybe add src address/endpoint to the callback?
};

struct query_t {
  uint16_t id;
  void (*callback)(uint16_t id, const cn_cbor *args);
};

command_t commands[MAX_COMMANDS];
query_t queries[MAX_QUERIES];
uint8_t commandCount = 0;
uint8_t queryIdCounter = 0;
uint8_t queryCounter = 0;
bool handle(uint16_t srcAddress, uint8_t srcEndpoint, const cn_cbor *args);

void reply(uint16_t address, uint16_t queryId, cn_cbor *args);

//
// Built-in commands
//

cn_cbor* reboot(const cn_cbor *args);
cn_cbor* otaMode(const cn_cbor *args);
cn_cbor* led(const cn_cbor *args);
cn_cbor* fuel(const cn_cbor *args);

void Commands::setup() {
  // Add our default commands
  add("otamode", otaMode);
  add("reboot", reboot);
  add("led", led);
  add("fuel", fuel);

  // Start listening on the command endpoint
  scout.mesh.listen(COMMAND_ENDPOINT, handle);
}

bool handle(uint16_t srcAddress, uint8_t srcEndpoint, const cn_cbor *data) {
  const cn_cbor *name = cn_cbor_mapget_string(data, "command");
  const cn_cbor *id = cn_cbor_mapget_string(data, "id");
  const cn_cbor *payload = cn_cbor_mapget_string(data, "args");

  bool found = false;
  if (name == NULL && id != NULL) { // Reply!
    for (uint8_t idx=0; idx<MAX_QUERIES; idx++) {
      if (queries[idx].id == id->v.uint) {
        found = true;
        queries[idx].callback(queries[idx].id, payload);
        queries[idx].id = NULL;
      }
    }
  } else { // Command!
    // Loop over commands to find it
    for (uint8_t idx=0; idx<MAX_COMMANDS; idx++) {
      if (strncmp(name->v.str, commands[idx].name, name->length) == 0) {
        found = true;
        cn_cbor *ret = commands[idx].handler(payload);

        // Check if we need to respond
        if (id != NULL) {
          reply(srcAddress, id->v.uint, ret);
        }
        break;
      }
    }
  }
  return found;

}

void reply(uint16_t address, uint16_t queryId, cn_cbor *args) {
  cn_cbor *wrapper = cn_cbor_map_create(NULL);
  cn_cbor_errback err;

  cn_cbor_mapput_string(wrapper, "id", cn_cbor_int_create(queryId, &err), NULL);
  cn_cbor_mapput_string(wrapper, "args", args, NULL);

  if (address == 0 || address == scout.settings.getAddress()) { //
    handle(0, COMMAND_ENDPOINT, wrapper);
  } else {
    scout.mesh.send(address, COMMAND_ENDPOINT, COMMAND_ENDPOINT, wrapper);
  }

  cn_cbor_free(wrapper);
}

bool Commands::add(const char* name, cn_cbor* (*handler)(const cn_cbor *args)) {
  // Check our ceiling for commands
  if (commandCount == MAX_COMMANDS) {
    return false;
  }

  uint8_t idx = commandCount++;
  commands[idx].name = strdup(name);
  commands[idx].handler = handler;
  return true;
}

void Commands::send(uint16_t address, const char *command, cn_cbor *args) {
    raw(address, command, args, NULL, NULL);
}

uint16_t Commands::query(uint16_t address, const char *command, cn_cbor *args, void (*callback)(uint16_t id, const cn_cbor *args)) {
    uint16_t queryId = ++queryIdCounter;
    if (queryId == NULL) {
      queryId = ++queryIdCounter;
    }
    return raw(address, command, args, queryId, callback);
}

uint16_t Commands::raw(uint16_t address, const char *command, cn_cbor *args, uint16_t queryId, void (*callback)(uint16_t id, const cn_cbor *args)) {
  cn_cbor *wrapper = cn_cbor_map_create(NULL);
  cn_cbor_errback err;

  cn_cbor_mapput_string(wrapper, "command", cn_cbor_string_create(command, &err), NULL);
  cn_cbor_mapput_string(wrapper, "args", args, NULL);

  if (queryId != NULL) {
    // Save the query callback
    uint8_t queryIdx = (queryCounter++ % MAX_QUERIES);
    queries[queryIdx].callback = callback;
    queries[queryIdx].id = queryId;
    cn_cbor_mapput_string(wrapper, "id", cn_cbor_int_create(queryId, &err), NULL);
  }

  if (address == 0 || address == scout.settings.getAddress()) { //
    handle(0, COMMAND_ENDPOINT, wrapper);
  } else {
    scout.mesh.send(address, COMMAND_ENDPOINT, COMMAND_ENDPOINT, wrapper);
  }

  cn_cbor_free(wrapper);

  return queryId;
}

cn_cbor* reboot(const cn_cbor *args) {
  scout.reboot();
  return NULL;
}

cn_cbor* otaMode(const cn_cbor *args) {
  scout.settings.setOTAFlag();
  scout.reboot();
  return NULL;
}

cn_cbor* led(const cn_cbor *args) {
  if (args->length == 0) {
    // No args -> off
    scout.led.turnOff();
  } else {
    uint8_t red = (uint8_t)cn_cbor_mapget_string(args, "r")->v.uint;
    uint8_t green = (uint8_t)cn_cbor_mapget_string(args, "g")->v.uint;
    uint8_t blue = (uint8_t)cn_cbor_mapget_string(args, "b")->v.uint;
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

cn_cbor* fuel(const cn_cbor *args) {
  cn_cbor *data = cn_cbor_int_create(HAL_FuelGaugePercent(), NULL);
  return data;
}

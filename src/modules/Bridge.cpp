#include <arduino.h>
#include "lwm/nwk/nwk.h"

#include <string.h>
#include <Scout.h>
#include "Bridge.h"

using namespace pinoccio;

Bridge bridge;
String buffer;
uint16_t messageLength = 0;
uint8_t messageBytes = 0;

void Bridge::setup(Scout *scout) {
    scout->mesh.listen(REPORT_ENDPOINT, handleReport);
}

void Bridge::loop() {
    // See if we have commands on Serial
    while(true) {
        int c = Serial.read();
        if (c < 0)
            return;

        if (messageBytes ==0) {
            messageLength = c;
            messageBytes++;
        } else if (messageBytes == 1) {
            messageLength += (c * 256);
            messageBytes++;
        } else {
            buffer.concat((char)c);
        }

        if (buffer.length() == messageLength) {
            cn_cbor_errback err;
            const cn_cbor *message = cn_cbor_decode((uint8_t*)buffer.c_str(), buffer.length(), &err);
            if (message != NULL) {
                const cn_cbor *name = cn_cbor_mapget_string(message, "name");
                const cn_cbor *address = cn_cbor_mapget_string(message, "address");
                const cn_cbor *args = cn_cbor_mapget_string(message, "args");

                // Make a pretty C-string for CN_CBOR
                char nameStr[name->length+1];
                strncpy(nameStr, name->v.str, name->length);
                nameStr[name->length] = '\0';

                scout.sendCommand((uint16_t)address->v.uint, nameStr, (cn_cbor*)args);
            }
            buffer = "";
            messageLength = 0;
            messageBytes = 0;
        }
    }
}

bool Bridge::handleReport(uint16_t srcAddress, uint8_t srcEndpoint, const cn_cbor *data) {
    // Write to Serial
    cn_cbor *wrapper = cn_cbor_map_create(NULL);

    cn_cbor_mapput_string(wrapper, "address", cn_cbor_int_create(srcAddress, NULL), NULL);
    cn_cbor_mapput_string(wrapper, "endpoint", cn_cbor_int_create(srcEndpoint, NULL), NULL);
    cn_cbor_mapput_string(wrapper, "data", (cn_cbor*)data, NULL);

    uint8_t buffer[NWK_FRAME_MAX_PAYLOAD_SIZE+32]; // add a few bytes for our wrapper
    size_t length = cbor_encoder_write(buffer, 0, sizeof(buffer), wrapper);
    Serial.write(buffer, length);
    Serial.flush();

    // Since we add a wrapper, we will free it.
    cn_cbor_free(wrapper);
    return 1; // signal we freed the data
}
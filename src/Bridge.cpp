#include <arduino.h>
#include "lwm/nwk/nwk.h"

#include <Bridge.h>
#include <Scout.h>


using namespace pinoccio;

Bridge::Bridge() {
    available = false;
}


void Bridge::setup(Mesh mesh) {
    available = true;

    mesh.join((uint16_t)Scout::REPORT_GROUP);
    mesh.listen(Scout::REPORT_ENDPOINT, handleReport);
}

void Bridge::write(const cn_cbor* data) {
    // Write data to Serial in <size><data> format
    uint8_t buffer[NWK_FRAME_MAX_PAYLOAD_SIZE];
    size_t length = cbor_encoder_write(buffer, 0, sizeof(buffer), data);
    Serial.write(length);
    Serial.write(buffer, length);
    Serial.flush();
}

bool Bridge::handleReport(uint8_t srcAddress, uint8_t srcEndpoint, const cn_cbor *data) {
    write(data);
}

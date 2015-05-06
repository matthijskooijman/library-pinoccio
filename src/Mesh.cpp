#include <Mesh.h>

#include "lwm/phy/atmegarfr2.h"
#include "lwm/phy/phy.h"
#include "lwm/hal/hal.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"

using namespace pinoccio;

static NWK_DataReq_t dataReq;
static void dataReqConfirm(NWK_DataReq_t *req);
static bool dataReqRecv(NWK_DataInd_t *ind);
static bool (*listenPoints[16])(uint8_t srcAddress, uint8_t srcEndpoint, const cn_cbor *data);

void Mesh::setup(Settings settings) {

  SYS_Init();
  PHY_RandomReq();

  // Fetch radio settings from eeprom and set them
  uint8_t buffer[16];
  settings.getSecurityKey((uint8_t*)buffer);
  setSecurityKey(buffer);
  memset(buffer, 0x00, 16); // clear the memory again

  setRadio(settings.getAddress(), settings.getPanId(), settings.getRadioChannel());
  setPower(settings.getRadioPower());
  setDataRate(settings.getRadioDataRate());

}

void Mesh::loop() {
  SYS_TaskHandler();
}

void Mesh::setRadio(const uint16_t addressId, const uint16_t panId, const uint8_t channel) {
  NWK_SetAddr(addressId);
  NWK_SetPanId(panId);
  setChannel(channel);
  PHY_SetRxState(true);
}

void Mesh::setChannel(const uint8_t channel) {
  PHY_SetChannel(channel);
}

void Mesh::setPower(const uint8_t power) {
  /* Page 116 of the 256RFR2 datasheet
    0   3.5 dBm
    1   3.3 dBm
    2   2.8 dBm
    3   2.3 dBm
    4   1.8 dBm
    5   1.2 dBm
    6   0.5 dBm
    7  -0.5 dBm
    8  -1.5 dBm
    9  -2.5 dBm
    10 -3.5 dBm
    11 -4.5 dBm
    12 -6.5 dBm
    13 -8.5 dBm
    14 -11.5 dBm
    15 -16.5 dBm
  */
  PHY_SetTxPower(power);
}

void Mesh::setDataRate(const uint8_t theRate) {
  /* Page 123 of the 256RFR2 datasheet
    0   250 kb/s  | -100 dBm
    1   500 kb/s  |  -96 dBm
    2   1000 kb/s |  -94 dBm
    3   2000 kb/s |  -86 dBm
  */
  TRX_CTRL_2_REG_s.oqpskDataRate = theRate;
}

void Mesh::setSecurityKey(const uint8_t *key) {
  NWK_SetSecurityKey((uint8_t *)key);
}

void Mesh::resetSecurityKey(void) {
  const uint8_t buf[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  setSecurityKey(buf);
}

void Mesh::listen(uint8_t endpoint, bool (*handler)(uint8_t srcAddress, uint8_t srcEndpoint, const cn_cbor *data)) {
  listenPoints[endpoint] = handler;
  NWK_OpenEndpoint(endpoint, dataReqRecv);
}

static bool dataReqRecv(NWK_DataInd_t *ind) {
  const cn_cbor *cb;
  cn_cbor_errback err;
  cb = cn_cbor_decode(ind->data, ind->size, &err);
  listenPoints[ind->dstEndpoint](ind->srcAddr, ind->srcEndpoint, cb);
}

void Mesh::send(uint16_t address, uint8_t srcEndpoint, uint8_t dstEndpoint, cn_cbor* data, bool multicast) {
  // TODO overflow?
  uint8_t buffer[NWK_FRAME_MAX_PAYLOAD_SIZE];
  size_t length = cbor_encoder_write(buffer, 0, sizeof(buffer), data);

  dataReq.dstAddr = address;
  dataReq.dstEndpoint = dstEndpoint;
  dataReq.srcEndpoint = srcEndpoint;
  dataReq.options = NWK_OPT_ENABLE_SECURITY;
  if (multicast) {
      dataReq.options |= NWK_OPT_MULTICAST;
  }
  dataReq.data = buffer;
  dataReq.size = length;
  dataReq.confirm = dataReqConfirm;

  NWK_DataReq(&dataReq);
}

void Mesh::multicast(uint16_t groupAddress, uint8_t srcEndpoint, uint8_t dstEndpoint, cn_cbor* data) {
  send(groupAddress, srcEndpoint, dstEndpoint, data, true);
}

void Mesh::broadcast(uint8_t srcEndpoint, uint8_t dstEndpoint, cn_cbor* data) {
  send(NWK_BROADCAST_PANID, srcEndpoint, dstEndpoint, data);
}

static void dataReqConfirm(NWK_DataReq_t *req) {
  if (req->status == NWK_SUCCESS_STATUS) {
  } else {
    // Retry?
  }
}

void Mesh::join(uint16_t groupAddress) {
  if (!NWK_GroupIsMember(groupAddress)) {
    NWK_GroupAdd(groupAddress);
  }
}

void Mesh::leave(uint16_t groupAddress) {
  if (NWK_GroupIsMember(groupAddress)) {
    NWK_GroupRemove(groupAddress);
  }
}

bool Mesh::isMemberOf(uint16_t groupAddress) {
  return NWK_GroupIsMember(groupAddress);
}

uint16_t Mesh::getAddress() {
  return nwkIb.addr;
}

uint16_t Mesh::getPanId() {
  return nwkIb.panId;
}

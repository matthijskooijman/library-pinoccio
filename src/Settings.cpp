#include <Settings.h>

#include <avr/eeprom.h>

using namespace pinoccio;

// Address 8124 - 1 byte   - Temperature offset
// Address 8125 - 1 byte   - Initiate OTA flag
// Address 8126 - 1 byte   - Data rate
// Address 8127 - 3 bytes  - Torch color (R,G,B)
// Address 8130 - 32 bytes - HQ Token
// Address 8162 - 16 bytes - Security Key
// Address 8178 - 1 byte   - Transmitter Power
// Address 8179 - 1 byte   - Frequency Channel
// Address 8180 - 2 bytes  - Network Identifier/Troop ID
// Address 8182 - 2 bytes  - Network Address/Scout ID
// Address 8184 - 4 bytes  - Unique ID
// Address 8188 - 2 bytes  - HW family
// Address 8190 - 1 byte   - HW Version
// Address 8191 - 1 byte   - EEPROM Version

void Settings::setup() {

    /* HQ token
    for (int i=0; i<32; i++) {
      buffer[i] = eeprom_read_byte((uint8_t *)8130+i);
    }
    setHQToken((char *)buffer);
    memset(buffer, 0x00, 32);
    */

    if (eeprom_read_byte((uint8_t *) 8124) != 0xFF) {
        tempOffset = (int8_t) eeprom_read_byte((uint8_t *) 8124);
    }
}

uint8_t Settings::getRadioPower() {
    return eeprom_read_byte((uint8_t *) 8178);
}

uint8_t Settings::getRadioChannel() {
    return eeprom_read_byte((uint8_t *) 8179);
}

uint16_t Settings::getAddress() {
    return eeprom_read_word((uint16_t *) 8182);
}

uint16_t Settings::getPanId() {
    return eeprom_read_word((uint16_t *) 8180);
}

uint8_t Settings::getRadioDataRate() {
    return eeprom_read_byte((uint8_t *) 8126);
}

void Settings::getSecurityKey(uint8_t *buffer) {
    for (int i = 0; i < 16; i++) {
        buffer[i] = eeprom_read_byte((uint8_t *) 8162 + i);
    }
}

uint32_t Settings::getHwSerial() {
    return eeprom_read_dword((uint32_t *) 8184);
}

uint16_t Settings::getHwFamily() {
    return eeprom_read_word((uint16_t *) 8188);
}

uint8_t Settings::getHwVersion() {
    return eeprom_read_byte((uint8_t *) 8190);
}

uint8_t Settings::getEEPROMVersion() {
    return eeprom_read_byte((uint8_t *) 8191);
}

int8_t Settings::getTemperatureOffset(void) {
    return tempOffset;
}

void Settings::setTemperatureOffset(int8_t offset) {
    eeprom_update_byte((uint8_t *) 8124, (uint8_t) offset);
    tempOffset = offset;
}

void Settings::setOTAFlag() {
    eeprom_update_byte((uint8_t *) 8125, 0x00);
}

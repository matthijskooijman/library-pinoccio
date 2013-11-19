#include "crc.h"

/* Generic CRC implementation, based on code generated by pycrc, using:
 * pycrc --width=8  --xor-in=0 --reflect-in=false --reflect-out=false --xor-out=0  --algorithm=bit-by-bit-fast --generate=c
 * poly parameter is the poly in hexadecimal form, "rocksoft" style
 * (translating the polynomial into a binary number, dropping the
 * topmost 1-bit).
 */
uint8_t crc_update(const uint8_t poly, uint8_t crc, uint8_t data)
{
    unsigned int i;
    bool bit;

    for (i = 0x80; i > 0; i >>= 1) {
        bit = crc & 0x80;
        if (data & i) {
            bit = !bit;
        }
        crc <<= 1;
        if (bit) {
            crc ^= poly;
        }
    }
    return crc;
}

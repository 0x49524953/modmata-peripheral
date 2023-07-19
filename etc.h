#include "Arduino.h"
#include <util/crc16.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef ETC_H
#define ETC_H

static inline const uint16_t bswap16(const uint16_t w) {
    // GCC extended inline ARM assembly snippet for swapping the bytes of 
    // a word without using additional registers :3
    uint16_t copy = w;
    
    __asm__ (
        "eor %A0, %B0" "\n\t"
        "eor %B0, %A0" "\n\t"
        "eor %A0, %B0" "\n\t"
        : "=r" (copy)
        : "0" (copy)
    );

    return copy;
}

static inline const uint16_t crc16(const uint8_t * data, const size_t len) {
    uint16_t crc = 0xffff;
    for (int i=0; i < len; i++) {
        // it came free with your fucking xbox
        // https://github.com/avrdudes/avr-libc/blob/55e8cac69935657bcd3e4d938750960c757844c3/include/util/crc16.h#L113
        crc = _crc16_update(crc, data[i]);
    }

    return crc;
}

static inline const uint8_t * exceptionCodeArray(uint8_t value) {
    const uint8_t array[1] = {value};
    return array;
}

static inline uint8_t * wordToBytes(const uint16_t w) {
    uint8_t array[2] = {lowByte(w), highByte(w)};
    return array;
}

static inline const uint16_t wordAtOffset(const uint8_t * data, const unsigned int index) {
    // sizeof(data) >= 2 or else things break
    // equiv of ```makeWord(data[index], data[index+1])```, but without the bitshifting (just pointer arithmetic)
    return *(uint16_t *)(data + index);
}

static inline const uint8_t reverseBits(const uint8_t b) {
    uint8_t p = ((b & 0xaa) >> 1) | ((b & 0x55) << 1);
    p = ((p & 0xcc) >> 2) | ((p & 0x33) << 2);
    p = ((p & 0xf0) >> 4) | ((p & 0x0f) << 4);
    return p;
}

static const void printBytes(const uint8_t * b, size_t l) {
  for (int i = 0; i < l; i++) {
    Serial.print(b[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
}

static inline const uint8_t boolsToByte(const bool a[]) {
    return 0x80 * a[0] | 0x40 * a[1] | 0x20 * a[2] | 0x10 * a[3] | 0x8 * a[4] | 0x4 * a[5] | 0x2 * a[6] | 0x1 * a[7];
}

#endif
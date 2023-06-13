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

static inline const bool functionAvailable(const uint8_t code) {
    return (
        (code >= 0x1 && code <= 0x6) || (code == 0xF) || (code == 0x10) ||
        (code >= 0x41 && code <= 0x48) || (code >= 0x64 && code <= 0x6e)
    );
}

#endif
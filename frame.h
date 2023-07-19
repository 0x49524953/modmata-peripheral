#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>
#include "etc.h"

#ifndef MODBUS_FRAME_H
#define MODBUS_FRAME_H

typedef struct CoilArray {
    bool coils[8] = {0,0,0,0, 0,0,0,0};

    CoilArray(const uint8_t b) {
        uint8_t _temp = reverseBits(b);
        for (int i = 0; i < 8; i++) {
            coils[i] = bitRead(_temp, 0);
            _temp >>= 1;
        }
    }
};

typedef struct Result {
    uint8_t * DATA;
    size_t LEN;

    Result() : DATA(nullptr), LEN(0u) {}
    ~Result() { free(DATA); LEN = 0u; }

    // Construction operators

    const Result& operator= (const Result& assign) {
        if (this != &assign) {
            this->~Result();
            this->LEN = assign.LEN;
            this->DATA = (uint8_t *)calloc(assign.LEN, sizeof(uint8_t));
            memcpy(this->DATA, assign.DATA, assign.LEN);
        }

        return *this;
    }

    Result(const Result& copy) {
        this->~Result();
        this->LEN = copy.LEN;
        this->DATA = (uint8_t *)calloc(copy.LEN, sizeof(uint8_t));
        memcpy(this->DATA, copy.DATA, copy.LEN);
    }

    // Various other constructors for Modbus functions

    Result(const uint8_t function, const uint8_t exception) 
    : LEN(2u), 
      DATA((uint8_t *)calloc(2u, sizeof(uint8_t))) {
        DATA[0] = function + 0x80;
        DATA[1] = exception;
    }

    Result(const uint8_t func, const uint8_t size, const uint8_t * data)
    : LEN(2u + size),
      DATA((uint8_t *)calloc(size + 2u, sizeof(uint8_t))) {
        DATA[0] = func;
        DATA[1] = size;
        memcpy(DATA+2, data, (size_t)size);
    }

    Result(const uint8_t func, const uint16_t addr, const uint16_t amt)
    : LEN(5u),
      DATA((uint8_t *)calloc(5u, sizeof(uint8_t))) {
        DATA[0] = func;
        DATA[1] = highByte(addr);
        DATA[2] = lowByte(addr);
        DATA[3] = highByte(amt);
        DATA[4] = lowByte(amt);
    }
};

typedef struct PDU_T {
    uint8_t * data;
    size_t len;

    PDU_T() : data(nullptr), len(0u) {}
    PDU_T(uint8_t * d, const size_t l) : data(d), len(l) {}
    ~PDU_T() { data = nullptr; len = 0; }

    uint8_t * codePtr() const { return data+0; }
    const uint8_t getCode() const { return *codePtr(); }
    const void setCode(const uint8_t code) { *codePtr() = code; }
};

typedef struct ADU_T {
    uint8_t * data;
    size_t len;
    PDU_T pdu;

    ADU_T() : data(nullptr), len(0u), pdu() {}
    ADU_T(uint8_t * d, const size_t l) : data(d), len(l), pdu(d+1, l-1) {}
    ~ADU_T() { data = nullptr; len = 0; pdu.~PDU_T(); }

    uint8_t * addrPtr() const { return data+0; }
    const uint8_t getAddr() const { return *addrPtr(); }
    const void setAddr(const uint8_t addr) { *addrPtr() = addr; }
};

typedef struct Packet {
    uint8_t * data;
    size_t len;
    ADU_T adu;

    Packet() : data(nullptr), len(0u), adu() {}

    Packet(const size_t l) {
        data = (uint8_t *)calloc(l, sizeof(uint8_t));
        len = l;
        adu = ADU_T(data, l - 2);
    }

    const void resetPacket() { memset(data+1, 0u, len-1); }
    uint16_t * crcPtr() const { return (uint16_t *)(data + (len - 2)); }
    const void updateCRC() { *crcPtr() =  crc16(data, len-2); }
    const uint16_t calcCRC() const { return crc16(data, len-2); }
    const bool checkCRC() const { return *crcPtr() == calcCRC(); }
    const void updateFromResult(const Result& r) {
        this->resetPacket();
        this->len = r.LEN + 3;
        memcpy(data + 1, r.DATA, r.LEN);
        this->updateCRC();
    }
    const void print() const { printBytes(this->data, this->len); }
};

#endif
/*
    ModbusSerial.h - Header for ModbusSerial Library
    Copyright (C) 2014 André Sarmento Barbosa
*/

#include <Arduino.h>
#include <Stream.h>
#include "Modbus.h"

#ifndef MODBUSSERIAL_H
#define MODBUSSERIAL_H

//#define USE_SOFTWARE_SERIAL

#ifdef USE_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
#endif

enum RX_STATE {
    STATE_RXERROR = 1,
    STATE_BADCRC = 2,
    STATE_NOTRECIPIENT = 3,
    STATE_BADFUNCTION = 4,
    STATE_TIMEOUT = 5,
    STATE_BROADCAST = 6,
    STATE_NORMAL = 7,
};

typedef struct PDU {
    uint8_t     CODE = 0;
    uint8_t *   DATA = nullptr;   // DO NOT calloc/malloc() FOR THIS!
    size_t      LEN  = 0;

    PDU() {}

    PDU(const uint8_t func, const uint8_t except) {
        uint8_t exc_array[1] = {except};   
        CODE = func + 0x80;
        DATA = exc_array;
        LEN = 1u;
    }

    const void use_crc_struct(uint8_t * crc_struct, const size_t len) {
        CODE = crc_struct[1];
        DATA = crc_struct + 2;
        LEN = len;
    }

};


class RTU_ADU {
    /**
     * i really wish i could define a variable-size struct for this since the
     * only unknown size is the length of the 'data' segment
     * 
     * i.e.
     *     uint8_t bytearray[] = {0x01, 0x04, 0x02, 0xFF, 0xFF, 0xB8, 0x80};
     *     ADU frame = bytearray;
     * 
     * or something... 
     * 
     * ADU/Frame format for ModbusRTU: [ https://en.wikipedia.org/wiki/Modbus#Modbus_RTU_frame_format ]
     * Structure of a Modbus RTU packet/message
     * - (28 bits of 'silence' that we ignore - it's a timed delay in other code, nothing is recv'd)
     * - 8-bit (uint8_t) address
     * - 8-bit (uint8_t) function code
     * - N-bytes (uint8_t *) of data
     * - 16-bit (uint16_t) CRC
     * - (28 bits of 'silence' that we ignore, again)
     */

    public:
        uint8_t * data =        nullptr;
        uint8_t * crc_struct =  nullptr;
        uint8_t * address =     nullptr;
        uint16_t * crc =        nullptr;
        size_t len =            0u;
        size_t crc_struct_len = 0u;

        PDU pdu = PDU();

        const size_t allocateGivenDataLen(const size_t len) {
            // calloc() helper
            this->len = len;
            this->crc_struct_len = len - 2;
            data = (uint8_t *)calloc(len, sizeof(uint8_t));
            crc_struct = data;
            address = data;
            crc = (uint16_t *)(data + (len-2));
            return len-2;
        }

        RTU_ADU() {}

        RTU_ADU(const uint8_t * d, const size_t len) {
            // ADU constructor for a 'len' byte long byte-array starting at 'd'
            // 01 04 02 FF FF B8 80
            // 0  1  2  3  4  5  6 ... 7 bytes long
            // +0 +1 +2 +3 +4 +5 +6
            this->allocateGivenDataLen(len);
            memcpy(data, d, len);
            pdu.use_crc_struct(data, len);
        }

        ~RTU_ADU() {
            if (data != nullptr) free(data);
            crc_struct = nullptr;
            address = nullptr;
            crc = nullptr;

            len = 0U;
            crc_struct_len = 0u;
            pdu = PDU();
        }

        const RTU_ADU& operator= (const RTU_ADU& assign) {
            // Assignment constructor
            // i.e. ADU frame = ADU(&Serial);
            // where    'assign' == 'ADU(&Serial)'
            // and      'this' == 'ADU frame'

            if (this != &assign) {
                this->~RTU_ADU();
                this->len = assign.len;
                this->crc_struct_len = assign.crc_struct_len;
                memcpy(data, assign.data, len);

                this->update();
            }

            return *this;
        }

        RTU_ADU(const RTU_ADU& copy) {
            this->~RTU_ADU();
            this->len = copy.len;
            this->crc_struct_len = copy.crc_struct_len;
            memcpy(data, copy.data, copy.len);
            this->update();
        }

        const bool checkCRC() const {
            return (*crc) == crc16(crc_struct, crc_struct_len);
        }

        const void update() {
            crc_struct = data;
            crc_struct_len = len - 2;
            address = data;
            crc = (uint16_t *)(data + (len - 2));

            pdu.use_crc_struct(crc_struct, crc_struct_len);
        }
};

class SerialModmata : public ModmataPeripheral {
    protected:
        Stream& serialStream;
        unsigned int serialFormat;
        unsigned long serialBaudRate;
        unsigned long serialTimeout = 1000;

        unsigned long _t = 0;
        // do we need a tx pin for other boards?

        RX_STATE packetState;

    public:
        uint8_t peripheralId;
        RTU_ADU currentPacket;

        SerialModmata(Stream& stream, unsigned long baud, unsigned int fmt) : serialStream(stream) {
            serialBaudRate = baud;
            serialFormat = fmt;
        };

        const bool          config(Stream& stream, unsigned long baud, unsigned int fmt);
        const void          setID(const uint8_t ID) { this->peripheralId = ID; }
        const uint8_t       getID() const { return peripheralId; }
        const void          setFormat(const unsigned int fmt) { this->serialFormat = fmt; }
        const unsigned int  getFormat() { return serialFormat; }
        const void          setBaud(const unsigned long baud) { this->serialBaudRate = baud; }
        const unsigned long getBaud() { return serialBaudRate; }
        const void          setStream(Stream& stream) { serialStream = stream; }

        const void startTimer() {_t = millis();}
        const bool timedOut() {return millis() - _t < serialTimeout;}

        const FunctionStruct& execute();
        const RX_STATE rxADU();
        //const bool txADU(const PDU_TYPE t); // (working on this)
};

#endif // MODBUSSERIAL_H

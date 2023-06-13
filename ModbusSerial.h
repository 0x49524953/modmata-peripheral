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

const enum RX_STATE {
    STATE_RXERROR = 0,
    STATE_BADCRC = 1,
    STATE_NOTRECIPIENT = 2,
    STATE_BADFUNCTION = 3,
    STATE_TIMEOUT = 4,
    STATE_BROADCAST = 5,
    STATE_NORMAL = 6,
};

typedef struct PDU {
    uint8_t     CODE;
    uint8_t *   DATA;   // DO NOT calloc/malloc() FOR THIS!
    size_t      LEN;
};

typedef struct RequestPDU : public PDU {
    const void use_crc_struct(const uint8_t * crcstruct, const size_t size) {
        CODE = crcstruct[1];
        LEN = size - sizeof(uint8_t) - sizeof(uint8_t) - sizeof(uint16_t);
                     // address         function code           CRC
        DATA = (uint8_t *)(crcstruct) + 2;
    }
};

typedef struct ResponsePDU : public PDU {};
typedef struct ExceptionPDU : public PDU {};


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
        uint8_t * _crc_struct = nullptr;
        uint16_t * crc =        nullptr;
        size_t _crc_size =      0;

        uint8_t address;
        RequestPDU pdu;

        RTU_ADU() {}

        RTU_ADU(const size_t len) {
            _crc_size = this->allocateGivenDataLen(len);
        }

        RTU_ADU(const uint8_t * d, const size_t len) {
            // ADU constructor for a 'len' byte long byte-array starting at 'd'
            // 01 04 02 FF FF B8 80
            // 0  1  2  3  4  5  6 ... 7 bytes long
            // +0 +1 +2 +3 +4 +5 +6
            _crc_size = this->allocateGivenDataLen(len);
            memcpy(_crc_struct, d, _crc_size);
            memcpy((uint8_t *)crc, d+_crc_size, 2);
            
            address = _crc_struct[0];
            pdu.use_crc_struct(_crc_struct, _crc_size);
        }
            
        const size_t allocateGivenDataLen(const size_t len) {
            // calloc() helper
            crc = (uint16_t *)calloc(1, sizeof(uint16_t));
            _crc_struct = (uint8_t *)calloc(len-2, sizeof(uint8_t));
            return len-2;
        }

        ~RTU_ADU() {
            if (_crc_struct != nullptr) free(_crc_struct);
            if (crc != nullptr) free(crc);
            _crc_struct = nullptr;
            crc = nullptr;

            _crc_size = 0U;
            address = 0U;
            pdu = RequestPDU();
        }

        const RTU_ADU& operator= (const RTU_ADU& assign) {
            // Assignment constructor
            // i.e. ADU frame = ADU(&Serial);
            // where    'assign' == 'ADU(&Serial)'
            // and      'this' == 'ADU frame'

            if (this != &assign) {
                this->~RTU_ADU();
                _crc_size =   this->allocateGivenDataLen(assign._crc_size+2);
                memcpy(_crc_struct, assign._crc_struct, assign._crc_size);
                *(crc) = *(assign.crc);
                this->update();
            }

            return *this;
        }

        RTU_ADU(const RTU_ADU& copy) {
            this->~RTU_ADU();
            _crc_size = this->allocateGivenDataLen(copy._crc_size+2);
            memcpy(_crc_struct, copy._crc_struct, copy._crc_size);
            *(crc) = *(copy.crc);
            this->update();
        }

        const bool checkCRC() const {
            return (*crc) == crc16(_crc_struct, _crc_size);
        }

        const void update() {
            address = _crc_struct[0];
            pdu.use_crc_struct(_crc_struct, _crc_size);
        }
};

class SerialModmata : public ModmataPeripheral {
    protected:
        Stream& serialStream = Serial;
        unsigned int serialFormat;
        unsigned long serialBaudRate;
        // do we need a tx pin?

    public:
        uint8_t peripheralId;
        RTU_ADU currentPacket;

        // This is being pissy
        SerialModmata(Stream& stream, unsigned long baud, unsigned int fmt) {
            this->serialStream = stream; // <- pisser
            this->serialBaudRate = baud;
            this->serialFormat = fmt;
        };

        const bool          config(Stream& stream, unsigned long baud, unsigned int fmt);
        const void          setID(const uint8_t ID) { this->peripheralId = ID; }
        const uint8_t       getID() const { return peripheralId; }
        const void          setFormat(const unsigned int fmt) { this->serialFormat = fmt; }
        const unsigned int  getFormat() { return serialFormat; }
        const void          setBaud(const unsigned long baud) { this->serialBaudRate = baud; }
        const unsigned long getBaud() { return serialBaudRate; }
        const void          setStream(Stream& stream) { serialStream = stream; }

        const PDU& execute();
        const RX_STATE rxADU(unsigned int timeout=0);
        const bool txADU();
        const void runOnce();
};

#endif // MODBUSSERIAL_H

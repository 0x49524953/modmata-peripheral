/*
    Modbus.h - Header for Modbus Base Library
    Copyright (C) 2014 André Sarmento Barbosa
*/

#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <util/crc16.h>
#include <math.h>

#include "registers.h"
#include "constants.h"
#include "etc.h"

#ifndef MODBUS_H
#define MODBUS_H

// ???
#define MAX_REGS     32
#define MAX_FRAME   128
//#define USE_HOLDING_REGISTERS_ONLY

typedef struct FunctionStruct {
    // Bare minimum data structure that applies to all Modbus ADU/PDU variants
    size_t      dataLen;
    uint8_t     functionCode;
    uint8_t *   functionData;
};


typedef struct MultiRead {
    size_t      valuesLen;
    uint16_t    initialAddress;
    uint16_t    numAddresses;
    uint16_t *  values;
};


class ModmataPeripheral {
    public:
        RegisterArray   table;

        ModmataPeripheral() {}

        // Basic Modbus functionality

        const FunctionStruct& ReadCoil(         const uint16_t address                          ) const;
        const FunctionStruct& ReadCoils(        const uint16_t address, const uint16_t amount   ) const;
        const FunctionStruct& ReadDiscrete(     const uint16_t address                          ) const;
        const FunctionStruct& ReadDiscretes(    const uint16_t address, const uint16_t amount   ) const;
        const FunctionStruct& ReadHolding(      const uint16_t address                          ) const;
        const FunctionStruct& ReadHoldings(     const uint16_t address, const uint16_t amount   ) const;
        const FunctionStruct& ReadInput(        const uint16_t address                          ) const;
        const FunctionStruct& ReadInputs(       const uint16_t address, const uint16_t amount   ) const;

        const FunctionStruct& WriteCoil(        const uint16_t address, const uint16_t value);
        const FunctionStruct& WriteCoils(       const uint16_t address, const uint16_t amount, const uint8_t * values);
        const FunctionStruct& WriteHolding(     const uint16_t address, const uint16_t value);
        const FunctionStruct& WriteHoldings(    const uint16_t address, const uint16_t amount, const uint16_t * values);

        // Extended functions

        virtual const void  WireBeginPeripheral(uint8_t address);
        virtual const void  WireBeginController();
        virtual const void  WireClock(uint32_t clock);
        virtual const void  WireRead() {};
        virtual const void  WireWrite() {};
        virtual const void  WireEnd() {};

        virtual const void  SPIbegin() {};
        virtual const void  SPIsettings() {};
        virtual const void  SPItransfer() {};
        virtual const void  SPIend() {};

    protected:

        // Convert a range of 1-bit registers to a byte 
        const uint8_t rangeToByte(const uint16_t address, const uint8_t count) const {
            uint8_t rval = 0;
            
            // Loop through each bit in a byte
            for (int i = 0; i < (count - 1); i++) {
                // One bit represents one register (Coil or Discrete)
                const Register ** r = table.getRegisterPtr(address + i);

                // Write state of register to the return value
                if (validRegister(r)) {
                    // bin(0x80) == 0b1000_0000 (OR'ing sets high bit to 1)
                    // set high bit if the value is non-zero, i guess
                    rval |= ((**r).value ? 0x80 : 0);
                }

                // right-shift (Low-index registers are LSB, Un-initialized registers treated as 0's)
                rval >>= 1;
            }

            // right-shift to zero-out high bits that we did not read
            return rval >> (8 - count);
        }

        uint8_t * rangeToBytes_OneBit(const uint16_t address, const uint16_t count) const {
            uint8_t f = count / 8;
            uint8_t r = count % 8;
            uint8_t size = f + bool(r);
            uint8_t returnArray[size] = {0,};

            uint16_t currentAddress = address;

            for (int i=0; (i < f && count >= 8); (i++, currentAddress += 8)) {
                returnArray[i] = rangeToByte(address, 8);
            }

            if (r) {
                returnArray[count-1] = rangeToByte(currentAddress, r);
            }

            return returnArray;
        }

        uint16_t * rangeToBytes_SixteenBit(const uint16_t address, const uint16_t count) const {
            uint16_t returnArray[count] = {0,};

            for (int i=0; i < count; i++) {
                const Register ** r = table.getRegisterPtr(address + i + 1);

                if (!validRegister(r))  returnArray[i] = 0u;
                else                    returnArray[i] = (**r).value;
            }

            return returnArray;
        }

    private:

};

const FunctionStruct& makeException(uint8_t functionCode, uint8_t exceptionCode) {
    const uint8_t oneByteArray[1] = {exceptionCode};
    return FunctionStruct{(size_t)1, (uint8_t)(functionCode + 128), (uint8_t *)(oneByteArray)};
}


#endif // MODBUS_H

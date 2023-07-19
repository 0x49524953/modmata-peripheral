/*
    Modbus.h - Header for Modbus Base Library
    Copyright (C) 2014 André Sarmento Barbosa
*/

#include "Arduino.h"

// If you want to use different I2C/SPI libraries, you may replace these
// headers as long as it has drop-in replacement compatability
#include <Wire.h>
#include <SPI.h>
//        ^^^^-- these 

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "registers.h"
#include "frame.h"
#include "constants.h"
#include "etc.h"

#ifndef MODBUS_H
#define MODBUS_H

// ???
#define MAX_REGS     32
#define MAX_FRAME   128

//#define USE_HOLDING_REGISTERS_ONLY

enum I2C_MODE {
    MODE_CONTROLLER = 1u,
    MODE_PERIPHERAL = 2u
};

class ModmataPeripheral {
    public:
        RegisterArray   table;
        SPISettings     spi_settings;
        Packet          lastPacket{Packet(255)};

        ModmataPeripheral() {}

        // Basic Modbus functionality

        const Result ReadCoil(         const uint16_t address                          ) const;
        const Result ReadCoils(        const uint16_t address, const uint16_t amount   ) const;
        const Result ReadDiscrete(     const uint16_t address                          ) const;
        const Result ReadDiscretes(    const uint16_t address, const uint16_t amount   ) const;
        const Result ReadHolding(      const uint16_t address                          ) const;
        const Result ReadHoldings(     const uint16_t address, const uint16_t amount   ) const;
        const Result ReadInput(        const uint16_t address                          ) const;
        const Result ReadInputs(       const uint16_t address, const uint16_t amount   ) const;

        const Result WriteCoil(        const uint16_t address, const uint16_t value);
        const Result WriteCoils(       const uint16_t address, const uint16_t amount, const uint8_t * values);
        const Result WriteHolding(     const uint16_t address, const uint16_t value);
        const Result WriteHoldings(    const uint16_t address, const uint16_t amount, const uint16_t * values);

        // Extended functions
        const Result PinMode(          const uint8_t pin, const uint8_t mode);
        const Result DigitalRead(      const uint8_t pin) const;
        const Result AnalogRead(       const uint8_t pin) const;
        const Result DigitalWrite(     const uint8_t pin, const uint8_t value);
        const Result AnalogWrite(      const uint8_t pin, const uint16_t value);

        const void printThing(const Result& r) {
            Serial.println("---");
            lastPacket.updateFromResult(r);
            lastPacket.print();
            this->table.printRegisters();
            Serial.println("---");
        }

    protected:

        // Convert a range of 1-bit registers to a byte (Read Coils, Discretes)
        const uint8_t rangeToByte(const uint16_t address, const uint8_t count) const {
            bool vals[8] = {0,0,0,0,0,0,0,0};

            for (int i=0; i < (count); i++) {
                vals[i] = table.getRegisterVal(address + i);
            }
            
            return boolsToByte(vals);
        }

};

#endif // MODBUS_H

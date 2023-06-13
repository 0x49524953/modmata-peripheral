/*
    Modbus.cpp - Source for Modbus Base Library
    Copyright (C) 2014 André Sarmento Barbosa
*/

#include "Modbus.h"

const FunctionStruct& ModmataPeripheral::ReadCoils(const uint16_t address, const uint16_t amount) const {

    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 7000);
    const bool ILLEGAL_ADDRESS = !(address >= 0  && address <= 9998 && address + amount <= 9998);

    if (ILLEGAL_VALUE)      return makeException(MB_FC_READ_COILS, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS)    return makeException(MB_FC_READ_COILS, MB_EX_ILLEGAL_ADDRESS);

    uint8_t floorByteSize = amount / 8;
    uint8_t remainder = amount % 8;
    uint8_t byteSize = floorByteSize + bool(remainder);

    uint8_t coils[byteSize] = {0,};
    uint16_t currentRegister = address+1;

    // Loop through registers, 8 at a time (amount >= 8)
    for (uint8_t index = 0; (index < floorByteSize && amount >= 8); (index++, currentRegister+=8)) {
        coils[index] = rangeToByte(currentRegister, 8);
    }

    // Last byte in array (if remainder > 0)
    if (bool(amount % 8)) {
        coils[byteSize-1] = rangeToByte(currentRegister, amount % 8);
    }

    return FunctionStruct{byteSize, MB_FC_READ_COILS, coils};
}

const FunctionStruct& ModmataPeripheral::ReadCoil(const uint16_t address) const {
    return this->ReadCoils(address, 1);
}

const FunctionStruct& ModmataPeripheral::ReadDiscretes(const uint16_t address, const uint16_t amount) const {
    // Essentially the same as Coils but with different codes and ranges
    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 7000);
    const bool ILLEGAL_ADDRESS = !(address >= 10000  && address <= 19998 && address + amount <= 19998);

    if (ILLEGAL_VALUE)      return makeException(MB_FC_READ_DISCRETES, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS)    return makeException(MB_FC_READ_DISCRETES, MB_EX_ILLEGAL_ADDRESS);

    uint8_t floorByteSize = amount / 8;
    uint8_t remainder = amount % 8;
    uint8_t byteSize = floorByteSize + bool(remainder);

    uint8_t coils[byteSize] = {0,};
    uint16_t currentRegister = address+1;

    // Loop through registers, 8 at a time (amount >= 8)
    for (uint8_t index = 0; (index < floorByteSize && amount >= 8); (index++, currentRegister+=8)) {
        coils[index] = rangeToByte(currentRegister, 8);
    }

    // Last byte in array (if remainder > 0)
    if (bool(amount % 8)) {
        coils[byteSize-1] = rangeToByte(currentRegister, amount % 8);
    }

    return FunctionStruct{byteSize, MB_FC_READ_DISCRETES, coils};
}

const FunctionStruct& ModmataPeripheral::ReadCoil(const uint16_t address) const {
    return this->ReadDiscretes(address, 1);
}

const FunctionStruct& ModmataPeripheral::ReadHoldings(const uint16_t address, const uint16_t amount) const {
    
}

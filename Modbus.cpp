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

    uint8_t f = amount / 8; // floor
    uint8_t r = amount % 8; // remainder
    uint8_t size = f + bool(r);

    return FunctionStruct{(size_t)size, MB_FC_READ_COILS, rangeToBytes_OneBit(address + 1, amount)};
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

    uint8_t f = amount / 8; // floor
    uint8_t r = amount % 8; // remainder
    uint8_t size = f + bool(r);

    return FunctionStruct{(size_t)size, MB_FC_READ_DISCRETES, rangeToBytes_OneBit(address + 1, amount)};
}

const FunctionStruct& ModmataPeripheral::ReadDiscrete(const uint16_t address) const {
    return this->ReadDiscretes(address, 1);
}

const FunctionStruct& ModmataPeripheral::ReadHoldings(const uint16_t address, const uint16_t amount) const {
    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 125);
    const bool ILLEGAL_ADDRESS = !(address >= 40000 && address <= 49998 && address + amount <= 49998);

    if (ILLEGAL_VALUE) return makeException(MB_FC_READ_HOLDINGS, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS) return makeException(MB_FC_READ_HOLDINGS, MB_EX_ILLEGAL_ADDRESS);

    return FunctionStruct{(size_t)(amount * 2), MB_FC_READ_HOLDINGS, (uint8_t *)rangeToBytes_SixteenBit(address, amount)};
}

const FunctionStruct& ModmataPeripheral::ReadHolding(const uint16_t address) const {
    return this->ReadHoldings(address, 1);
}

const FunctionStruct& ModmataPeripheral::ReadInputs(const uint16_t address, const uint16_t amount) const {
    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 125);
    const bool ILLEGAL_ADDRESS = !(address >= 30000 && address <= 39998 && address + amount <= 39998);

    if (ILLEGAL_VALUE) return makeException(MB_FC_READ_INPUTS, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS) return makeException(MB_FC_READ_INPUTS, MB_EX_ILLEGAL_ADDRESS);

    return FunctionStruct{(size_t)(amount * 2), MB_FC_READ_INPUTS, (uint8_t *)rangeToBytes_SixteenBit(address, amount)};
}

const FunctionStruct& ModmataPeripheral::ReadInput(const uint16_t address) const {
    return this->ReadInputs(address, 1);
}

const FunctionStruct& ModmataPeripheral::WriteCoil(const uint16_t address, const uint16_t value) {
    const bool ILLEGAL_VALUE = !(value == 0xFF00 || value == 0x0000);
    const bool ILLEGAL_ADDRESS = !(address >= 0 && address <= 9998);

    if (ILLEGAL_VALUE) return makeException(MB_FC_WRITE_COIL, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS) return makeException(MB_FC_WRITE_COIL, MB_EX_ILLEGAL_ADDRESS);

    if (table.verifySetRegister(address + 1, value)) 
        return FunctionStruct{(size_t)2u, MB_FC_WRITE_COIL, wordToBytes(value)};

    return makeException(MB_FC_WRITE_COIL, MB_EX_DEVICE_FAILURE);    
}

const FunctionStruct& ModmataPeripheral::WriteCoils(const uint16_t address, const uint16_t amount, const uint8_t * values) {
    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 7000);
    const bool ILLEGAL_ADDRESS = !(address >= 0  && address <= 9998 && address + amount <= 9998);
    bool DEVICE_FAIL = false;

    if (ILLEGAL_VALUE)      return makeException(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS)    return makeException(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_ADDRESS);

    uint8_t f = amount / 8;
    uint8_t r = amount % 8;
    uint8_t size = f + bool(r);

    uint16_t currentRegister = address + 1;

    for (int i=0; (i < f && f >= 1); i++) {
        // each byte sent
        uint8_t curByte = values[i];

        for (int j=0; j < 8; (j++, currentRegister++, curByte>>=1)) {
            // 0xFF00 if low bit is 1, 0x0000 if it is 0
            const uint16_t asWord = 0xFF00 * (curByte & 1u);
            const bool REGISTER_SET = table.verifySetRegister(currentRegister, asWord);
            if (!REGISTER_SET) DEVICE_FAIL = true;
        }
    }

    if (DEVICE_FAIL) return makeException(MB_FC_WRITE_COILS, MB_EX_DEVICE_FAILURE);

    return FunctionStruct{(size_t)2u, MB_FC_WRITE_COILS, wordToBytes(amount)};
}

const FunctionStruct& ModmataPeripheral::WriteHolding(const uint16_t address, const uint16_t value) {
    const bool ILLEGAL_ADDRESS = !(address >= 40000 && address <= 49998 && address <= 49998);

    if (ILLEGAL_ADDRESS) return makeException(MB_FC_WRITE_HOLDING, MB_EX_ILLEGAL_ADDRESS);

    // set value
    const bool REGISTER_SET = table.verifySetRegister(address + 1, value);
    if (!REGISTER_SET) return makeException(MB_FC_WRITE_HOLDING, MB_EX_DEVICE_FAILURE);

    return FunctionStruct{(size_t)2u, MB_FC_WRITE_HOLDING, wordToBytes(value)};
}

const FunctionStruct& ModmataPeripheral::WriteHoldings(const uint16_t address, const uint16_t amount, const uint16_t * values) {
    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 125);
    const bool ILLEGAL_ADDRESS = !(address >= 40000 && address <= 49998 && address + amount <= 49998);
    bool DEVICE_FAIL = false;

    if (ILLEGAL_VALUE) return makeException(MB_FC_WRITE_HOLDINGS, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS) return makeException(MB_FC_WRITE_HOLDINGS, MB_EX_ILLEGAL_ADDRESS);

    for (int i=0; i < amount; i++) {
        const bool REGISTER_SET = table.verifySetRegister(address + i + 1, values[i]);
        if (!REGISTER_SET) DEVICE_FAIL = true;
    }

    if (DEVICE_FAIL) return makeException(MB_FC_WRITE_COILS, MB_EX_DEVICE_FAILURE);

    return FunctionStruct{(size_t)2u, MB_FC_WRITE_HOLDINGS, wordToBytes(amount)};
}
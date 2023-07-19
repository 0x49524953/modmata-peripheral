/*
    Modbus.cpp - Source for Modbus Base Library
    Copyright (C) 2014 André Sarmento Barbosa
*/

#include "Modbus.h"

/**
 * @brief Read multiple (sequential) Modbus coil registers
 * 
 * @param address Initial address to read from
 * @param amount Number of registers to read
 * @return const Result&
 */
const Result ModmataPeripheral::ReadCoils(const uint16_t address, const uint16_t amount) const {

    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 7000);
    const bool ILLEGAL_ADDRESS = !(address >= 0  && address <= 9998 && address + amount <= 9998);

    if (ILLEGAL_VALUE)      return Result(MB_FC_READ_COILS, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS)    return Result(MB_FC_READ_COILS, MB_EX_ILLEGAL_ADDRESS);

    const uint8_t f = amount / 8; // floor
    const uint8_t r = amount % 8; // remainder
    const uint8_t size = f + bool(r);

    uint8_t array[size] = {0,};
    uint16_t currentAddress = address + 1;
    uint16_t numRegs = amount;

    for (int i=0; i < size; i++) {
        int regsThisLoop = (numRegs >= 8) ? 8 : numRegs;
        array[i] = reverseBits(rangeToByte(currentAddress + i*8, regsThisLoop));
        numRegs -= regsThisLoop;
    }

    return Result(MB_FC_READ_COILS, size, array);
}

/**
 * @brief Read a single Modbus coil register
 * 
 * @param address Register to read from
 * @return const Result& 
 */
const Result ModmataPeripheral::ReadCoil(const uint16_t address) const {
    return this->ReadCoils(address, 1);
}

// SHOULD WORK
/**
 * @brief Read multiple (sequential) Modbus discrete registers
 * 
 * @param address Initial address to read from
 * @param amount Number of registers to read
 * @return const Result& 
 */
const Result ModmataPeripheral::ReadDiscretes(const uint16_t address, const uint16_t amount) const {
    // Essentially the same as Coils but with different codes and ranges
    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 7000);
    const bool ILLEGAL_ADDRESS = !(address >= 10000  && address <= 19998 && address + amount <= 19998);

    if (ILLEGAL_VALUE)      return Result(MB_FC_READ_DISCRETES, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS)    return Result(MB_FC_READ_DISCRETES, MB_EX_ILLEGAL_ADDRESS);

    const uint8_t f = amount / 8; // floor
    const uint8_t r = amount % 8; // remainder
    const uint8_t size = f + bool(r);

    uint8_t array[size] = {0,};
    uint16_t currentAddress = address + 1;
    uint16_t numRegs = amount;

    for (int i=0; i < size; i++) {
        int regsThisLoop = (numRegs >= 8) ? 8 : numRegs;
        array[i] = reverseBits(rangeToByte(currentAddress + i*8, regsThisLoop));
        numRegs -= regsThisLoop;
    }

    return Result(MB_FC_READ_DISCRETES, size, array);
}

// SHOULD WORK
/**
 * @brief Read a single Modbus discrete register
 * 
 * @param address Register to read from
 * @return const Result& 
 */
const Result ModmataPeripheral::ReadDiscrete(const uint16_t address) const {
    return this->ReadDiscretes(address, 1);
}

/**
 * @brief Read multiple (sequential) Modbus holding registers
 * 
 * @param address Initial address to read from
 * @param amount Number of registers to read
 * @return const Result&
 */
const Result ModmataPeripheral::ReadHoldings(const uint16_t address, const uint16_t amount) const {
    const uint16_t actual_addr = address + 40001;

    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 125);
    const bool ILLEGAL_ADDRESS = !(actual_addr >= 40001 && actual_addr <= 49999 && actual_addr + amount <= 49999);

    if (ILLEGAL_VALUE) return Result(MB_FC_READ_HOLDINGS, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS) return Result(MB_FC_READ_HOLDINGS, MB_EX_ILLEGAL_ADDRESS);

    uint8_t size = amount * 2;
    uint8_t array[size] = {0,};

    for (int i=0; i < amount; i++) {
        const Register ** r = table.getRegisterPtr(actual_addr + i);

        if (validRegister(r)) {
            uint16_t wrd = (**r).value;
            array[   i*2   ] = highByte(wrd);
            array[ (i*2)+1 ] = lowByte(wrd);
        }

        else {
            array[   i*2   ] = 0u;
            array[ (i*2)+1 ] = 0u;
        }                   
    }

    return Result(MB_FC_READ_HOLDINGS, size, array);
}

/**
 * @brief Read a single Modbus holding register
 * 
 * @param address Register to read from
 * @return const Result& 
 */
const Result ModmataPeripheral::ReadHolding(const uint16_t address) const {
    return this->ReadHoldings(address, 1);
}

// SHOULD WORK
const Result ModmataPeripheral::ReadInputs(const uint16_t address, const uint16_t amount) const {
    const uint16_t actual_addr = address + 40001;

    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 125);
    const bool ILLEGAL_ADDRESS = !(actual_addr >= 30001 && actual_addr <= 39999 && actual_addr + amount <= 39999);

    if (ILLEGAL_VALUE) return Result(MB_FC_READ_INPUTS, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS) return Result(MB_FC_READ_INPUTS, MB_EX_ILLEGAL_ADDRESS);

    uint8_t size = amount * 2;
    uint8_t array[size] = {0,};

    for (int i=0; i < amount; i++) {
        const Register ** r = table.getRegisterPtr(actual_addr + i);

        if (validRegister(r)) {
            uint16_t wrd = (**r).value;
            array[   i*2   ] = highByte(wrd);
            array[ (i*2)+1 ] = lowByte(wrd);
        }

        else {
            array[   i*2   ] = 0u;
            array[ (i*2)+1 ] = 0u;
        }                   
    }

    return Result(MB_FC_READ_INPUTS, size, array);
}

// SHOULD WORK
const Result ModmataPeripheral::ReadInput(const uint16_t address) const {
    return this->ReadInputs(address, 1);
}

const Result ModmataPeripheral::WriteCoil(const uint16_t address, const uint16_t value) {
    const uint16_t actual_addr = address+1;

    const bool ILLEGAL_VALUE = !(value == 0xFF00 || value == 0x0000);
    const bool ILLEGAL_ADDRESS = !(actual_addr >= 1 && actual_addr <= 9999);

    if (ILLEGAL_VALUE) return Result(MB_FC_WRITE_COIL, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS) return Result(MB_FC_WRITE_COIL, MB_EX_ILLEGAL_ADDRESS);

    if (table.verifySetRegister(actual_addr, value))
        return Result(MB_FC_WRITE_COIL, actual_addr, value);

    return Result(MB_FC_WRITE_COIL, MB_EX_DEVICE_FAILURE);    
}

const Result ModmataPeripheral::WriteCoils(const uint16_t address, const uint16_t amount, const uint8_t * values) {
    const uint16_t actual_addr = address+1;

    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 7000);
    const bool ILLEGAL_ADDRESS = !(actual_addr >= 1  && actual_addr <= 9999 && actual_addr + amount <= 9999);
    bool DEVICE_FAIL = false;

    if (ILLEGAL_VALUE)      return Result(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS)    return Result(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_ADDRESS);

    const uint16_t reg = actual_addr;
    uint16_t numCoils = amount;

    const uint8_t byteCount = values[0];
    const uint8_t * coilVals = values + 1;

    for (int i=0; i < byteCount; i++) {
        // each byte sent
        CoilArray ca(coilVals[i]);

        const int coilsInLoop = (numCoils >= 8) ? 8 : numCoils;

        for (int j=0; j < coilsInLoop; j++) {
            const uint16_t curRegister = reg + ((i * 8) + j);
            const uint16_t asWord = 0xFF00 * ca.coils[7-j];

            const bool REGISTER_SET = table.verifySetRegister(curRegister, asWord);
            if (!REGISTER_SET) DEVICE_FAIL = true;
        }

        numCoils -= 8;
    }

    if (DEVICE_FAIL) return Result(MB_FC_WRITE_COILS, MB_EX_DEVICE_FAILURE);

    return Result(MB_FC_WRITE_COILS, address, amount);
}

const Result ModmataPeripheral::WriteHolding(const uint16_t address, const uint16_t value) {
    const uint16_t actual_addr = address + 40001;

    const bool ILLEGAL_ADDRESS = !(actual_addr >= 40001 && actual_addr <= 49999);
    if (ILLEGAL_ADDRESS) return Result(MB_FC_WRITE_HOLDING, MB_EX_ILLEGAL_ADDRESS);

    // set value
    const bool REGISTER_SET = table.verifySetRegister(actual_addr, value);
    if (!REGISTER_SET) return Result(MB_FC_WRITE_HOLDING, MB_EX_DEVICE_FAILURE);

    return Result(MB_FC_WRITE_HOLDING, address, value);
}


const Result ModmataPeripheral::WriteHoldings(const uint16_t address, const uint16_t amount, const uint16_t * values) {
    const uint16_t actual_addr = address + 40001;

    const bool ILLEGAL_VALUE = !(amount >= 1 && amount <= 125);
    const bool ILLEGAL_ADDRESS = !(actual_addr >= 40001 && actual_addr <= 49999 && actual_addr + amount <= 49999);
    bool DEVICE_FAIL = false;

    if (ILLEGAL_VALUE) return Result(MB_FC_WRITE_HOLDINGS, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS) return Result(MB_FC_WRITE_HOLDINGS, MB_EX_ILLEGAL_ADDRESS);

    for (int i=0; i < amount; i++) {
        const bool REGISTER_SET = table.verifySetRegister(actual_addr + i, bswap16(values[i]));
        if (!REGISTER_SET) DEVICE_FAIL = true;
    }

    if (DEVICE_FAIL) return Result(MB_FC_WRITE_COILS, MB_EX_DEVICE_FAILURE);

    return Result(MB_FC_WRITE_HOLDINGS, address, amount);
}

// TODO
const Result ModmataPeripheral::PinMode(const uint8_t pin, uint8_t mode) {
    const bool ILLEGAL_VALUE = (mode != INPUT && mode != INPUT_PULLUP && mode != OUTPUT);
    
    if (ILLEGAL_VALUE) return Result(MB_FC_PINMODE, MB_EX_ILLEGAL_VALUE);

    pinMode(pin, mode);

    return Result(MB_FC_PINMODE, pin, mode);
}

// TODO
const Result ModmataPeripheral::DigitalRead(const uint8_t pin) const {
    const uint16_t val = (0xFF00 * digitalRead(pin));

    return Result(MB_FC_DIGITAL_READ, pin, val);
}

// TODO
const Result ModmataPeripheral::AnalogRead(const uint8_t pin) const {
    const uint16_t val = analogRead(pin);

    return Result(MB_FC_ANALOG_READ, pin, val);
}

// TODO
const Result ModmataPeripheral::DigitalWrite(const uint8_t pin, const uint8_t value) {
    const bool ILLEGAL_VALUE = (value != HIGH && value != LOW);
    const bool ILLEGAL_ADDRESS = (digitalPinToPort(pin) == NOT_A_PIN);

    if (ILLEGAL_VALUE) return Result(MB_FC_DIGITAL_WRITE, MB_EX_ILLEGAL_VALUE);
    if (ILLEGAL_ADDRESS) return Result(MB_FC_DIGITAL_WRITE, MB_EX_ILLEGAL_ADDRESS);

    digitalWrite(pin, value);

    return Result(MB_FC_DIGITAL_WRITE, pin, value);
}

// TODO
const Result ModmataPeripheral::AnalogWrite(const uint8_t pin, const uint16_t value) {
    const bool ILLEGAL_ADDRESS = (pin == NOT_A_PIN);

    if (ILLEGAL_ADDRESS) return Result(MB_FC_ANALOG_WRITE, MB_EX_ILLEGAL_ADDRESS);

    analogWrite(pin, (int)value);

    return Result(MB_FC_ANALOG_WRITE, pin, value);
}


/*
    ModbusSerial.cpp - Source for Modbus Serial Library
    Copyright (C) 2014 André Sarmento Barbosa
*/

#include "ModbusSerial.h"
#include "constants.h"

const bool SerialModmata::config(Stream& stream, unsigned long baud, unsigned int fmt) {
    this->serialStream = stream;
    this->serialFormat = fmt;
    this->serialBaudRate = baud;
}

const FunctionStruct& SerialModmata::execute() {
    switch (currentPacket.pdu.CODE) {
        // i think this is a bit easier to understand?
        // 'wordAtOffset()' equivalent to 'bswap16(*(uint16_t *)(currentPacket.pdu.DATA + i));'
        // use bswap16 because little-endian is LSB first and Modbus sends data MSB first (save for the CRC)
        
        // also yes I know these 'break's are mostly redundant, I'm just trying to be careful in case 
        // return doesn't work like it usually does

        case MB_FC_READ_COILS: {
            uint16_t address = bswap16(wordAtOffset(currentPacket.pdu.DATA, 0));
            uint16_t amount = bswap16(wordAtOffset(currentPacket.pdu.DATA, 2));
            return ReadCoils(address, amount);
            break;
        }

        case MB_FC_READ_DISCRETES: {
            uint16_t address = 10000 + bswap16(wordAtOffset(currentPacket.pdu.DATA, 0));
            uint16_t amount = bswap16(wordAtOffset(currentPacket.pdu.DATA, 2));
            return ReadDiscretes(address, amount);
            break;
        }

        case MB_FC_READ_HOLDINGS: {
            uint16_t address = 40000 + bswap16(wordAtOffset(currentPacket.pdu.DATA, 0));
            uint16_t amount = bswap16(wordAtOffset(currentPacket.pdu.DATA, 2));
            return ReadHoldings(address, amount);
            break;
        }

        case MB_FC_READ_INPUTS: {
            uint16_t address = 30000 + bswap16(wordAtOffset(currentPacket.pdu.DATA, 0));
            uint16_t amount = bswap16(wordAtOffset(currentPacket.pdu.DATA, 2));
            return ReadInputs(address, amount);
            break;
        }

        case MB_FC_WRITE_COIL: {
            uint16_t address = bswap16(wordAtOffset(currentPacket.pdu.DATA, 0));
            uint16_t value = bswap16(wordAtOffset(currentPacket.pdu.DATA, 2));
            return WriteCoil(address, value);
            break;
        }

        case MB_FC_WRITE_HOLDING: {
            uint16_t address = 40000 + bswap16(wordAtOffset(currentPacket.pdu.DATA, 0));
            uint16_t value = bswap16(wordAtOffset(currentPacket.pdu.DATA, 2));
            return WriteHolding(address, value);
            break;
        }

        case MB_FC_WRITE_COILS: {
            uint16_t startAddress = bswap16(wordAtOffset(currentPacket.pdu.DATA, 0));
            uint16_t amount = bswap16(wordAtOffset(currentPacket.pdu.DATA, 2));
            uint8_t * values = currentPacket.pdu.DATA + 5;
            return WriteCoils(startAddress, amount, values);
            break;
        }

        case MB_FC_WRITE_HOLDINGS: {
            uint16_t startAddress = 40000 + bswap16(wordAtOffset(currentPacket.pdu.DATA, 0));
            uint16_t amount = bswap16(wordAtOffset(currentPacket.pdu.DATA, 2));
            uint16_t * values = (uint16_t *)(currentPacket.pdu.DATA + 5);
            return WriteHoldings(startAddress, amount, values);
            break;
        }

        case MB_FC_PINMODE: {
            uint8_t pin = currentPacket.pdu.DATA[0];
            uint8_t mode = currentPacket.pdu.DATA[1];
            return PinMode(pin, mode);
            break;
        }

        case MB_FC_DIGITAL_READ: {
            uint8_t pin = currentPacket.pdu.DATA[0];
            return DigitalRead(pin);
            break;
        }

        case MB_FC_DIGITAL_WRITE: {
            uint8_t pin = currentPacket.pdu.DATA[0];
            uint8_t value = currentPacket.pdu.DATA [1];
            return DigitalWrite(pin, value);
            break;
        }

        case MB_FC_ANALOG_READ: {
            uint8_t pin = currentPacket.pdu.DATA[0];
            return AnalogRead(pin);
            break;
        }

        case MB_FC_ANALOG_WRITE: {
            uint8_t pin = currentPacket.pdu.DATA[0];
            uint16_t value = wordAtOffset(currentPacket.pdu.DATA, 1);
            return AnalogWrite(pin, value);
            break;
        }

        default: {
            return makeException(currentPacket.pdu.CODE, MB_EX_ILLEGAL_FUNCTION);
            break;
        }
    }
}

const RX_STATE SerialModmata::rxADU() {
    size_t bytes_waiting = 0;
    startTimer();
    while ((!bytes_waiting) && (!timedOut())) {
        bytes_waiting = serialStream.available();
    }

    if (!bytes_waiting)                                 return STATE_TIMEOUT;
    
    uint8_t * chunk = (uint8_t *)calloc(bytes_waiting, sizeof(uint8_t));
    const size_t read_b = serialStream.readBytes(chunk, bytes_waiting);
    
    if (read_b != bytes_waiting) { free(chunk);         return STATE_RXERROR; }
    
    this->currentPacket = RTU_ADU(chunk, read_b);
    free(chunk);

    // Basic checks
    if (!currentPacket.checkCRC())                      return STATE_BADCRC;
    if (*currentPacket.address == 0x0)                  return STATE_BROADCAST;
    if (*currentPacket.address != getID())              return STATE_NOTRECIPIENT;
    if (!functionAvailable(currentPacket.pdu.CODE))     return STATE_BADFUNCTION;
    return STATE_NORMAL;
}

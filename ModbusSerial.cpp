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

const RX_STATE SerialModmata::rxADU(unsigned int timeout=0) {
    size_t bytes_waiting = 0;
    while (this->serialStream.available() > bytes_waiting)
        bytes_waiting = this->serialStream.available();

    if (!bytes_waiting) return STATE_RXERROR; // idk what happened

    // Recv with timeout capabilities
    uint8_t * chunk = (uint8_t *)calloc(bytes_waiting, sizeof(uint8_t));
    const size_t read = this->serialStream.readBytes(chunk, bytes_waiting);
    if (read != bytes_waiting) { free(chunk);       return STATE_TIMEOUT; }
    this->currentPacket = RTU_ADU(chunk, read);
    free(chunk);

    // Basic checks
    if (!currentPacket.checkCRC())                  return STATE_BADCRC;
    if (currentPacket.address == 0x0)               return STATE_BROADCAST;
    if (currentPacket.address != getID())           return STATE_NOTRECIPIENT;
    if (!functionAvailable(currentPacket.pdu.CODE)) return STATE_BADFUNCTION;

    return STATE_NORMAL;
}

const PDU& SerialModmata::execute() {

}

const bool SerialModmata::txADU() {}

const void SerialModmata::runOnce() {
    const uint8_t status = this->rxADU();


}

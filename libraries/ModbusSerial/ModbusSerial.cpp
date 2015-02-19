/*
    ModbusSerial.cpp - Source for Modbus Serial Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
*/
#include "ModbusSerial.h"

ModbusSerial::ModbusSerial() {

}

bool ModbusSerial::config(HardwareSerial* port, long baud, u_int format, int txPin) {
    this->_port = port;
    this->_txPin = txPin;
    (*port).begin(baud, format);

    delay(2000);

    if (txPin >= 0) {
        pinMode(txPin, OUTPUT);
        digitalWrite(txPin, LOW);
    }

    if (baud > 19200) {
		_t15 = 750;
		_t35 = 1750;
	} else {
		_t15 = 15000000/baud; // 1T * 1.5 = T1.5
		_t35 = 35000000/baud; // 1T * 3.5 = T3.5
	}

    return true;
}

bool ModbusSerial::receive(byte* frame) {
    //first byte of frame = address
    byte address = frame[0];
    //Last two bytes = crc
    u_int crc = ((frame[_len - 2] << 8) | frame[_len - 1]);

    //Slave Check
    if (address != 0xFF && address != this->getSlaveId()) {
    	free(_frame);
	    _len = 0;
		return false;
	}

    //CRC Check
    if (crc != this->calcCrc(_frame[0], _frame+1, _len-3)) {
        free(_frame);
	    _len = 0;
		return false;
    }

    //PDU starts after first byte
    //framesize PDU = framesize - address(1) - crc(2)
    this->receivePDU(frame+1);

    return true;
}

bool ModbusSerial::send(byte* frame) {
    byte i;
    for (i = 0 ; i < _len ; i++) {
        (*_port).write(frame[i]);
    }
}

bool ModbusSerial::send(byte address, byte* pduframe) {
    //Send slaveId
    (*_port).write(address);

    //Send PDU
    byte i;
    for (i = 0 ; i < _len ; i++) {
        (*_port).write(pduframe[i]);
    }

    //Send CRC
    word crc = calcCrc(address, _frame, _len);
    (*_port).write(crc >> 8);
    (*_port).write(crc & 0xFF);
}

void ModbusSerial::proc() {
    _len = 0;

    while ((*_port).available()> _len)	{
		_len = (*_port).available();
		delayMicroseconds(_t35);
		//delayMicroseconds(1250);
	}

	if (_len == 0) return;

	byte i;
	_frame = (byte*) malloc(_len);
	for (i=0 ; i < _len ; i++) _frame[i] = (*_port).read();

    this->receive(_frame);

    if (_reply == REPLY_NORMAL) this->send(this->getSlaveId(), _frame);
    if (_reply == REPLY_ECHO) this->send(_frame);

  	free(_frame);
	_len = 0;
}

word ModbusSerial::calcCrc(byte address, byte* pduFrame, byte pduLen) {
	byte CRCHi = 0xFF, CRCLo = 0x0FF, Index;

    Index = CRCHi ^ address;
    CRCHi = CRCLo ^ _auchCRCHi[Index];
    CRCLo = _auchCRCLo[Index];

	while (pduLen--) {
		Index = CRCHi ^ *pduFrame++;
		CRCHi = CRCLo ^ _auchCRCHi[Index];
		CRCLo = _auchCRCLo[Index];
	}

	return (CRCHi << 8) | CRCLo;
}





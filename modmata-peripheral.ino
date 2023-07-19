#include "ModbusSerial.h"

Packet packet(255);
ModmataPeripheral mp;

void setup() {
    mp = ModmataPeripheral();
    packet.adu.setAddr(0x11);
    
    //sm.setID(0x11);        
//    pinMode(13, OUTPUT);
}

void loop() {

//    const RX_STATE state = sm.rxADU();
//    if (state == STATE_NORMAL) {
//        sm.execute();
//        digitalWrite(13, sm.ReadCoil(1).DATA[2]);
//    }
    
}

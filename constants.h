#ifndef CONSTANTS_H
#define CONSTANTS_H

// Function Codes
enum FUNCTIONS {
    MB_FC_READ_COILS            = 0x01, // Read Coils (Output) Status           0xxxx
    MB_FC_READ_DISCRETES        = 0x02, // Read Input Status (Discrete Inputs)  1xxxx
    MB_FC_READ_HOLDINGS         = 0x03, // Read Holding Registers               4xxxx
    MB_FC_READ_INPUTS           = 0x04, // Read Input Registers                 3xxxx
    MB_FC_WRITE_COIL            = 0x05, // Write Single Coil (Output)           0xxxx
    MB_FC_WRITE_HOLDING         = 0x06, // Preset Single Register               4xxxx
    MB_FC_WRITE_COILS           = 0x0F, // Write Multiple Coils (Outputs)       0xxxx
    MB_FC_WRITE_HOLDINGS        = 0x10, // Write block of contiguous registers  4xxxx

    // vvv-- subject to change as I work on them --vvv

    // Basic IO manipulation
    MB_FC_PINMODE               = 0x41, // pinMode
	MB_FC_DIGITAL_READ	        = 0x42, // digitalRead
	MB_FC_DIGITAL_WRITE	        = 0x43, // digitalWrite
    MB_FC_ANALOG_READ           = 0x44, // analogRead
    MB_FC_ANALOG_WRITE          = 0x45, // analogWrite

    // I2C
    /**
    MB_FC_WIRE_BEGIN_PERIPHERAL = 0x46, // Wire.begin(address)
    MB_FC_WIRE_BEGIN_CONTROLLER = 0x47, // Wire.begin()
    MB_FC_WIRE_END              = 0x64, // Wire.end()
    MB_FC_WIRE_CLK              = 0x65, // Wire.setClock()
    MB_FC_WIRE_READ             = 0x66, // Wire.readBytes()
    MB_FC_WIRE_WRITE            = 0x67, // Wire.write()???

    // SPI
    MB_FC_SPI_BEGIN             = 0x67, // SPI.begin()
    MB_FC_SPI_END               = 0x68, // SPI.end()
    MB_FC_SPI_SETTINGS          = 0x69, // SPISettings
    MB_FC_SPI_TRANSFER          = 0x70, // SPI.transfer()

	//MB_FC_USR_CALLBACK		= 107,	// user-defined callback (maybe)
    **/
};

// Exception Codes
enum EXCEPTIONS {
    MB_EX_ILLEGAL_FUNCTION = 0x01,  // Function Code Not Supported
    MB_EX_ILLEGAL_ADDRESS  = 0x02,  // Given Address Not In Acceptable Range
    MB_EX_ILLEGAL_VALUE    = 0x03,  // Given Value Not In Acceptable Range
    MB_EX_DEVICE_FAILURE    = 0x04, // Arduino Fails To Process Request
};

// Reply Types
enum REPLY_T {
    MB_REPLY_OFF    = 0x01,
    MB_REPLY_ECHO   = 0x02,
    MB_REPLY_NORMAL = 0x03,
};

// Register Types
enum REGISTER_T {
    MB_REGISTER_COIL =      0x1,
    MB_REGISTER_DISCRETE =  0x2,
    MB_REGISTER_INPUT =     0x3,
    MB_REGISTER_HOLDING =   0x4
};

#endif
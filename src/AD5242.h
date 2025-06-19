#pragma once

//    FILE: AD5242.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.2
// PURPOSE: I2C digital PotentioMeter AD5242
//    DATE: 2013-10-12
//     URL: https://github.com/RobTillaart/AD5242

#include <Arduino.h>
#include <Wire.h>


#define AD5242_LIB_VERSION    (F("0.1.2"))

#define AD5242_OK             0
#define AD5242_ERROR          100

#define AD5242_MIDPOINT       127

class AD5242 {
public:
  AD5242(const uint8_t address, TwoWire *wire = &Wire);

  bool    begin(const uint32_t PRating);
  bool    isConnected();
  uint8_t getAddress();
  uint8_t pmCount();

  // READ WRITE
  uint8_t read(const uint8_t rdac);
  uint32_t readResistance(const uint8_t rdac);
  uint8_t getResistancePercent(const uint8_t rdac);
  uint8_t write(const uint8_t rdac, const uint8_t value);
  uint8_t write(const uint8_t rdac, const uint8_t value, const uint8_t O1, const uint8_t O2);
  uint8_t writeResistance(const uint8_t rdac, const uint32_t value, const uint8_t O1, const uint8_t O2);

  // IO LINES
  uint8_t setO1(const uint8_t value = HIGH);  // HIGH (default) / LOW
  uint8_t setO2(const uint8_t value = HIGH);  // HIGH (default) / LOW
  bool getO1();
  bool getO2();

  // RESET
  uint8_t reset();    // reset both channels to AD5242_MIDPOINT and O1/O2 to LOW
  uint8_t zeroAll();  // set both channels to 0 and O1/O2 to LOW
  uint8_t midScaleReset(const uint8_t rdac);

  // DEBUGGING
  uint8_t readBackRegister();  // returns the last value written in register.

  // experimental - to be tested - use at own risk
  uint8_t shutDown();          // datasheet P15

private:
  uint32_t _potRating = 0;
  uint8_t _pmCount = 2;

  uint8_t send(const uint8_t cmd, const uint8_t value);

  uint8_t _address;
  uint8_t _lastValue[2];
  uint8_t _O1;
  uint8_t _O2;

  TwoWire * _wire;
};

//  -- END OF FILE --
//    FILE: AD5242.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.2
// PURPOSE: I2C digital potentiometer AD5242
//    DATE: 2013-10-12
//     URL: https://github.com/RobTillaart/AD5242

#include "AD5242.h"

#define AD5242_RDAC0        0x00
#define AD5242_RDAC1        0x80
#define AD5242_RESET        0x40
#define AD5242_SHUTDOWN     0x20
#define AD5242_O1_HIGH      0x10
#define AD5242_O2_HIGH      0x08

// address: 0x01011xx = 0x2C, 0x2D, 0x2E, or 0x2F
AD5242::AD5242(const uint8_t address, TwoWire *wire) : _address(address), _wire(wire), _O1(0), _O2(0), _pmCount(2) {
  _lastValue[0] = _lastValue[1] = AD5242_MIDPOINT;
}

bool AD5242::begin(const uint32_t PRating) {
  if (! isConnected()) return false;

  // Just allow 10k, 100k or 1M
  if (PRating != 10000 && PRating != 100000 && PRating != 1000000)
    return false;

  _potRating = PRating;

  reset();

  return true;
}

bool AD5242::isConnected() {
  _wire->beginTransmission(_address);
  return ( _wire->endTransmission() == 0);
}

uint8_t AD5242::getAddress() {
  return _address;
}

uint8_t AD5242::pmCount() {
  return _pmCount;
}


uint8_t AD5242::read(const uint8_t rdac) {
  if (rdac >= _pmCount) return AD5242_ERROR;
  return _lastValue[rdac];
}

uint32_t AD5242::readResistance(const uint8_t rdac) {
  if (rdac >= _pmCount) return AD5242_ERROR;
  return (_lastValue[rdac] * _potRating + 127) / 255; // Compute percent with rounding
}

uint8_t AD5242::getResistancePercent(const uint8_t rdac) {
  if (rdac >= _pmCount) return AD5242_ERROR;

  return (_lastValue[rdac] * 100 + 127) / 255; // Compute percent with rounding
}

uint8_t AD5242::write(const uint8_t rdac, const uint8_t value) {
  if (rdac >= _pmCount) return AD5242_ERROR;

  uint8_t cmd = (rdac == 0) ? AD5242_RDAC0 : AD5242_RDAC1;
  //  apply the output lines
  cmd = cmd | _O1 | _O2;
  _lastValue[rdac] = value;
  return send(cmd, value);
}

uint8_t AD5242::write(const uint8_t rdac, const uint8_t value, const uint8_t O1, const uint8_t O2) {
  if (rdac >= _pmCount) return AD5242_ERROR;

  _O1 = (O1 == LOW) ? 0 : AD5242_O1_HIGH;
  _O2 = (O2 == LOW) ? 0 : AD5242_O2_HIGH;

  uint8_t cmd = (rdac == 0) ? AD5242_RDAC0 : AD5242_RDAC1;
  //  apply the output lines
  cmd = cmd | _O1 | _O2;
  _lastValue[rdac] = value;
  return send(cmd, value);
}

uint8_t AD5242::writeResistance(const uint8_t rdac, const uint32_t value, const uint8_t O1, const uint8_t O2) {
  if (rdac >= _pmCount) return AD5242_ERROR;

  if (value >= _potRating) return AD5242_ERROR;

  uint8_t cResistance = (value * 255 + _potRating / 2) / _potRating;

  return write(rdac, cResistance, O1, O2);
}


uint8_t AD5242::setO1(const uint8_t value) {
  _O1 = (value == LOW) ? 0 : AD5242_O1_HIGH;
  uint8_t cmd = AD5242_RDAC0 | _O1 | _O2;
  return send(cmd, _lastValue[0]);
}

uint8_t AD5242::setO2(const uint8_t value) {
  _O2 = (value == LOW) ? 0: AD5242_O2_HIGH;
  uint8_t cmd = AD5242_RDAC0 | _O1 | _O2;
  return send(cmd, _lastValue[0]);
}

bool AD5242::getO1() { return _O1 > 0; }
bool AD5242::getO2() { return _O2 > 0; }

uint8_t AD5242::reset() {
  write(0, AD5242_MIDPOINT, LOW, LOW);
  return write(1, AD5242_MIDPOINT);
}

uint8_t AD5242::zeroAll() {
  write(0, 0, LOW, LOW);
  return write(1, 0);
}

uint8_t AD5242::midScaleReset(const uint8_t rdac) {
  if (rdac >= _pmCount) return AD5242_ERROR;

  uint8_t cmd = AD5242_RESET;
  if (rdac == 1) cmd |= AD5242_RDAC1;
  cmd = cmd | _O1 | _O2;
  _lastValue[rdac] = AD5242_MIDPOINT;
  return send(cmd, _lastValue[rdac]);
}

//  read datasheet P.15
uint8_t AD5242::shutDown() {
  uint8_t cmd = AD5242_SHUTDOWN;  // TODO TEST & VERIFY
  return send(cmd, 0);
}

uint8_t AD5242::readBackRegister() {
  _wire->beginTransmission(_address);
  _wire->endTransmission();
  if (_wire->requestFrom(_address, (uint8_t)1) != 1)
    return AD5242_ERROR;
  return _wire->read();
}


uint8_t AD5242::send(const uint8_t cmd, const uint8_t value) {
  _wire->beginTransmission(_address);
  _wire->write(cmd);
  _wire->write(value);
  return _wire->endTransmission();
}

//  -- END OF FILE --
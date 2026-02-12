//    FILE: AD5242.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.3.0
// PURPOSE: I2C digital potentiometer AD5242
//    DATE: 2013-10-12
//     URL: https://github.com/RobTillaart/AD5242

#include "AD5242.h"

static const uint8_t AD5242_RDAC0        = 0x00;
static const uint8_t AD5242_RDAC1        = 0x80;
static const uint8_t AD5242_RESET        = 0x40;
static const uint8_t AD5242_SHUTDOWN     = 0x20;
static const uint8_t AD5242_O1_HIGH      = 0x10;
static const uint8_t AD5242_O2_HIGH      = 0x08;

// address: 0x01011xx = 0x2C, 0x2D, 0x2E, or 0x2F
AD5242::AD5242(const uint8_t address, const uint8_t hwEnablePin, TwoWire *wire) :
  _address(address),
  _wire(wire),
  _lastValue{ AD5242_MIDPOINT, AD5242_MIDPOINT },
  _O1(0),
  _O2(0),
  _potRating(0),
  _potRatingAB{ 0, 0 },
  _hasRating(false),
  _hwEnablePin(outputMask(hwEnablePin) == 0 ? 0 : hwEnablePin),
  _lastStatus(AD5242_OK) {
}

AD5242Status AD5242::begin(const uint32_t potRating) {
  if (!isValidRating(potRating)) return setStatus(AD5242_ERR_PARAM);
  if (!isConnected()) return setStatus(AD5242_ERR_I2C);

  _potRating = potRating;
  _hasRating = true;

  if (_hwEnablePin != 0) {
    AD5242Status hwStatus = setO(_hwEnablePin, true);
    if (hwStatus != AD5242_OK) return setStatus(hwStatus);
  }

  return setStatus(reset());
}

bool AD5242::isConnected() const {
  _wire->beginTransmission(_address);
  return (_wire->endTransmission() == 0);
}

uint8_t AD5242::address() const {
  return _address;
}

uint8_t AD5242::channelCount() const {
  return kChannelCount;
}

bool AD5242::isInitialized() const {
  return _hasRating;
}

uint32_t AD5242::potRating() const {
  return _potRating;
}

AD5242Status AD5242::lastStatus() const {
  return _lastStatus;
}

uint8_t AD5242::hwEnablePin() const {
  return _hwEnablePin;
}

AD5242Status AD5242::setABRvalue(const uint8_t rdac, const uint32_t abResistance) {
  uint8_t idx = channelIndex(rdac);
  if (idx >= kChannelCount) return setStatus(AD5242_ERR_PARAM);
  if (abResistance == 0) return setStatus(AD5242_ERR_PARAM);
  _potRatingAB[idx] = abResistance;
  return setStatus(AD5242_OK);
}


uint8_t AD5242::getLastValue(const uint8_t rdac) {
  uint8_t idx = channelIndex(rdac);
  if (idx >= kChannelCount) {
    setStatus(AD5242_ERR_PARAM);
    return 0;
  }
  setStatus(AD5242_OK);
  return _lastValue[idx];
}

uint32_t AD5242::getResistance(const uint8_t rdac, const char direction) {
  const char dir = normalizeDirection(direction);
  if (dir == 0) {
    setStatus(AD5242_ERR_PARAM);
    return 0;
  }
  uint8_t idx = channelIndex(rdac);
  if (idx >= kChannelCount) {
    setStatus(AD5242_ERR_PARAM);
    return 0;
  }
  uint32_t rating = potRatingFor(idx);
  if (rating == 0) {
    setStatus(AD5242_ERR_NOT_INITIALIZED);
    return 0;
  }

  uint32_t bToW = (_lastValue[idx] * rating + (AD5242_MAX_VALUE / 2)) / AD5242_MAX_VALUE;
  setStatus(AD5242_OK);
  if (dir == 'A') {
    return rating - bToW;
  }
  return bToW;
}

uint32_t AD5242::getResistance(const uint8_t rdac, const char *direction) {
  return getResistance(rdac, normalizeDirection(direction));
}

uint8_t AD5242::getResistancePercent(const uint8_t rdac, const char direction) {
  const char dir = normalizeDirection(direction);
  if (dir == 0) {
    setStatus(AD5242_ERR_PARAM);
    return 0;
  }
  uint8_t idx = channelIndex(rdac);
  if (idx >= kChannelCount) {
    setStatus(AD5242_ERR_PARAM);
    return 0;
  }

  uint8_t percent = (_lastValue[idx] * 100 + (AD5242_MAX_VALUE / 2)) / AD5242_MAX_VALUE;
  setStatus(AD5242_OK);
  if (dir == 'A') {
    return 100 - percent;
  }
  return percent;
}

uint8_t AD5242::getResistancePercent(const uint8_t rdac, const char *direction) {
  return getResistancePercent(rdac, normalizeDirection(direction));
}

AD5242Status AD5242::write(const uint8_t rdac, const uint8_t value) {
  uint8_t idx = channelIndex(rdac);
  if (idx >= kChannelCount) return setStatus(AD5242_ERR_PARAM);

  uint8_t cmd = (idx == 0) ? AD5242_RDAC0 : AD5242_RDAC1;
  //  apply the output lines
  cmd = cmd | _O1 | _O2;
  AD5242Status status = send(cmd, value);
  if (status == AD5242_OK) {
    _lastValue[idx] = value;
  }
  return setStatus(status);
}

AD5242Status AD5242::write(const uint8_t rdac, const uint8_t value, const bool O1, const bool O2) {
  uint8_t idx = channelIndex(rdac);
  if (idx >= kChannelCount) return setStatus(AD5242_ERR_PARAM);

  const uint8_t newO1 = (O1) ? AD5242_O1_HIGH : 0;
  const uint8_t newO2 = (O2) ? AD5242_O2_HIGH : 0;

  uint8_t cmd = (idx == 0) ? AD5242_RDAC0 : AD5242_RDAC1;
  //  apply the output lines
  cmd = cmd | newO1 | newO2;
  AD5242Status status = send(cmd, value);
  if (status == AD5242_OK) {
    _O1 = newO1;
    _O2 = newO2;
    _lastValue[idx] = value;
  }
  return setStatus(status);
}

AD5242Status AD5242::writeResistance(const uint8_t rdac, const uint32_t value) {
  return writeResistance(rdac, value, _O1 > 0, _O2 > 0, 'B');
}

AD5242Status AD5242::writeResistance(const uint8_t rdac, const uint32_t value, const char direction) {
  return writeResistance(rdac, value, _O1 > 0, _O2 > 0, direction);
}

AD5242Status AD5242::writeResistance(const uint8_t rdac, const uint32_t value, const char *direction) {
  return writeResistance(rdac, value, _O1 > 0, _O2 > 0, normalizeDirection(direction));
}

AD5242Status AD5242::writeResistance(const uint8_t rdac, const uint32_t value, const bool O1, const bool O2) {
  return writeResistance(rdac, value, O1, O2, 'B');
}

AD5242Status AD5242::writeResistance(const uint8_t rdac, const uint32_t value, const bool O1, const bool O2, const char direction) {
  uint8_t idx = channelIndex(rdac);
  if (idx >= kChannelCount) return setStatus(AD5242_ERR_PARAM);
  uint32_t rating = potRatingFor(idx);
  if (rating == 0) return setStatus(AD5242_ERR_NOT_INITIALIZED);
  const char dir = normalizeDirection(direction);
  if (dir == 0) return setStatus(AD5242_ERR_PARAM);
  if (value > rating) return setStatus(AD5242_ERR_PARAM);

  uint32_t effective = (dir == 'A') ? (rating - value) : value;
  uint8_t cResistance = (effective * AD5242_MAX_VALUE + rating / 2) / rating;

  return setStatus(write(rdac, cResistance, O1, O2));
}

AD5242Status AD5242::writeResistance(const uint8_t rdac, const uint32_t value, const bool O1, const bool O2, const char *direction) {
  return writeResistance(rdac, value, O1, O2, normalizeDirection(direction));
}


AD5242Status AD5242::setO1(const bool value) {
  return setO(1, value);
}

AD5242Status AD5242::setO2(const bool value) {
  return setO(2, value);
}

bool AD5242::getO1() const { return _O1 > 0; }
bool AD5242::getO2() const { return _O2 > 0; }

AD5242Status AD5242::setO(const uint8_t output, const bool value) {
  uint8_t mask = outputMask(output);
  if (mask == 0) return setStatus(AD5242_ERR_PARAM);

  uint8_t newO1 = _O1;
  uint8_t newO2 = _O2;
  if (output == 1) newO1 = value ? AD5242_O1_HIGH : 0;
  if (output == 2) newO2 = value ? AD5242_O2_HIGH : 0;

  uint8_t cmd = AD5242_RDAC0 | newO1 | newO2;
  AD5242Status status = send(cmd, _lastValue[0]);
  if (status == AD5242_OK) {
    _O1 = newO1;
    _O2 = newO2;
  }
  return setStatus(status);
}

bool AD5242::getO(const uint8_t output) {
  uint8_t mask = outputMask(output);
  if (mask == 0) {
    setStatus(AD5242_ERR_PARAM);
    return false;
  }
  setStatus(AD5242_OK);
  return (output == 1) ? (_O1 > 0) : (_O2 > 0);
}

AD5242Status AD5242::HWEnable() {
  if (_hwEnablePin == 0) return setStatus(AD5242_ERR_PARAM);
  return setStatus(setO(_hwEnablePin, true));
}

AD5242Status AD5242::HWDisable() {
  if (_hwEnablePin == 0) return setStatus(AD5242_ERR_PARAM);
  return setStatus(setO(_hwEnablePin, false));
}

//  read datasheet P.15
AD5242Status AD5242::shutDown() {
  uint8_t cmd = AD5242_SHUTDOWN;  // TODO TEST & VERIFY
  return setStatus(send(cmd, 0));
}

AD5242Status AD5242::reset() {
  AD5242Status status = write(1, AD5242_MIDPOINT, false, false);
  if (status != AD5242_OK) return status;
  status = write(2, AD5242_MIDPOINT);
  if (status != AD5242_OK) return status;
  if (_hwEnablePin != 0) return setStatus(setO(_hwEnablePin, true));
  return status;
}

AD5242Status AD5242::zeroAll() {
  AD5242Status status = write(1, 0, false, false);
  if (status != AD5242_OK) return status;
  return write(2, 0);
}

AD5242Status AD5242::midScaleReset(const uint8_t rdac) {
  uint8_t idx = channelIndex(rdac);
  if (idx >= kChannelCount) return setStatus(AD5242_ERR_PARAM);

  uint8_t cmd = AD5242_RESET;
  if (idx == 1) cmd |= AD5242_RDAC1;
  cmd = cmd | _O1 | _O2;
  AD5242Status status = send(cmd, AD5242_MIDPOINT);
  if (status == AD5242_OK) {
    _lastValue[idx] = AD5242_MIDPOINT;
  }
  return setStatus(status);
}

AD5242Status AD5242::readBackRegister(uint8_t &value) {
  _wire->beginTransmission(_address);
  if (_wire->endTransmission() != 0) return setStatus(AD5242_ERR_I2C);
  if (_wire->requestFrom(_address, (uint8_t)1) != 1) return setStatus(AD5242_ERR_I2C);
  value = _wire->read();
  return setStatus(AD5242_OK);
}


AD5242Status AD5242::send(const uint8_t cmd, const uint8_t value) {
  _wire->beginTransmission(_address);
  _wire->write(cmd);
  _wire->write(value);
  return (_wire->endTransmission() == 0) ? AD5242_OK : AD5242_ERR_I2C;
}


bool AD5242::isValidRating(const uint32_t potRating) {
  return (potRating == AD5242_R10K || potRating == AD5242_R100K || potRating == AD5242_R1M);
}

uint8_t AD5242::channelIndex(const uint8_t rdac) {
  if (rdac == 1) return 0;
  if (rdac == 2) return 1;
  return 0xFF;
}

uint8_t AD5242::outputMask(const uint8_t output) {
  if (output == 1) return AD5242_O1_HIGH;
  if (output == 2) return AD5242_O2_HIGH;
  return 0;
}

char AD5242::normalizeDirection(const char direction) {
  if (direction == 'A' || direction == 'a') return 'A';
  if (direction == 'B' || direction == 'b') return 'B';
  return 0;
}

char AD5242::normalizeDirection(const char *direction) {
  if (direction == nullptr || direction[0] == '\0') return 0;
  return normalizeDirection(direction[0]);
}

uint32_t AD5242::potRatingFor(const uint8_t idx) const {
  if (idx >= kChannelCount) return 0;
  if (_potRatingAB[idx] != 0) return _potRatingAB[idx];
  return _potRating;
}

AD5242Status AD5242::setStatus(const AD5242Status status) {
  _lastStatus = status;
  return status;
}

//  -- END OF FILE --

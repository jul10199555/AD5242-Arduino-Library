//    FILE: AD5242.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.3.2
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
  _wiperResistance(AD5242_RW_DEFAULT),
  _endStopProtection(true),
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

AD5242Status AD5242::setWiperResistance(const uint16_t wiperResistance) {
  uint16_t value = wiperResistance;
  if (value < AD5242_RW_MIN) value = AD5242_RW_MIN;
  if (value > AD5242_RW_MAX) value = AD5242_RW_MAX;
  _wiperResistance = value;
  return setStatus(AD5242_OK);
}

uint16_t AD5242::getWiperResistance() const {
  return _wiperResistance;
}

void AD5242::setEndStopProtection(const bool enabled) {
  _endStopProtection = enabled;
}

bool AD5242::getEndStopProtection() const {
  return _endStopProtection;
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

  setStatus(AD5242_OK);
  return resistanceForCode(rating, _lastValue[idx], dir);
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

  uint16_t numerator = (dir == 'B') ? _lastValue[idx] : (256 - _lastValue[idx]);
  uint8_t percent = (numerator * 100 + 128) / 256;
  setStatus(AD5242_OK);
  return percent;
}

uint8_t AD5242::getResistancePercent(const uint8_t rdac, const char *direction) {
  return getResistancePercent(rdac, normalizeDirection(direction));
}

uint32_t AD5242::codeToResistance(const uint8_t rdac, const uint8_t code, const char direction) {
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

  setStatus(AD5242_OK);
  return resistanceForCode(rating, sanitizeCode(code), dir);
}

uint32_t AD5242::codeToResistance(const uint8_t rdac, const uint8_t code, const char *direction) {
  return codeToResistance(rdac, code, normalizeDirection(direction));
}

uint8_t AD5242::resistanceToCode(const uint8_t rdac, const uint32_t resistance, const char direction) {
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

  setStatus(AD5242_OK);
  return codeForResistance(rating, resistance, dir);
}

uint8_t AD5242::resistanceToCode(const uint8_t rdac, const uint32_t resistance, const char *direction) {
  return resistanceToCode(rdac, resistance, normalizeDirection(direction));
}

AD5242Status AD5242::write(const uint8_t rdac, const uint8_t value) {
  uint8_t idx = channelIndex(rdac);
  if (idx >= kChannelCount) return setStatus(AD5242_ERR_PARAM);
  uint8_t safeValue = sanitizeCode(value);

  uint8_t cmd = (idx == 0) ? AD5242_RDAC0 : AD5242_RDAC1;
  //  apply the output lines
  cmd = cmd | _O1 | _O2;
  AD5242Status status = send(cmd, safeValue);
  if (status == AD5242_OK) {
    _lastValue[idx] = safeValue;
  }
  return setStatus(status);
}

AD5242Status AD5242::write(const uint8_t rdac, const uint8_t value, const bool O1, const bool O2) {
  uint8_t idx = channelIndex(rdac);
  if (idx >= kChannelCount) return setStatus(AD5242_ERR_PARAM);
  uint8_t safeValue = sanitizeCode(value);

  const uint8_t newO1 = (O1) ? AD5242_O1_HIGH : 0;
  const uint8_t newO2 = (O2) ? AD5242_O2_HIGH : 0;

  uint8_t cmd = (idx == 0) ? AD5242_RDAC0 : AD5242_RDAC1;
  //  apply the output lines
  cmd = cmd | newO1 | newO2;
  AD5242Status status = send(cmd, safeValue);
  if (status == AD5242_OK) {
    _O1 = newO1;
    _O2 = newO2;
    _lastValue[idx] = safeValue;
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

  uint8_t maxCode = (dir == 'B') ? sanitizeCode(AD5242_MAX_VALUE) : sanitizeCode(0);
  uint32_t maxResistance = resistanceForCode(rating, maxCode, dir);
  if (value > maxResistance) return setStatus(AD5242_ERR_PARAM);

  uint8_t cResistance = codeForResistance(rating, value, dir);

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

uint8_t AD5242::sanitizeCode(const uint8_t value) const {
  if (!_endStopProtection) return value;
  if (value == 0) return 1;
  if (value == AD5242_MAX_VALUE) return AD5242_MAX_VALUE - 1;
  return value;
}

uint32_t AD5242::potRatingFor(const uint8_t idx) const {
  if (idx >= kChannelCount) return 0;
  if (_potRatingAB[idx] != 0) return _potRatingAB[idx];
  return _potRating;
}

uint32_t AD5242::resistanceForCode(const uint32_t rating, const uint8_t code, const char direction) const {
  uint32_t rw = _wiperResistance;
  if (direction == 'B') {
    // RWB(D) = D/256 * RAB + RW
    return (uint32_t)((((uint64_t)code * rating) + 128) / 256) + rw;
  }
  // RWA(D) = (256 - D)/256 * RAB + RW
  uint64_t upper = (uint64_t)(256 - code) * rating;
  return (uint32_t)((upper + 128) / 256) + rw;
}

uint8_t AD5242::codeForResistance(const uint32_t rating, const uint32_t resistance, const char direction) const {
  uint32_t rw = _wiperResistance;
  uint32_t effective = (resistance <= rw) ? 0 : (resistance - rw);
  uint64_t x64 = ((uint64_t)effective * 256 + rating / 2) / rating;  // x in equation space
  uint32_t x = (x64 > 256) ? 256 : (uint32_t)x64;

  uint8_t code = 0;
  if (direction == 'B') {
    if (x > 255) x = 255;
    code = (uint8_t)x;
  } else {
    uint32_t d = 256 - x;
    if (d > 255) d = 255;
    code = (uint8_t)d;
  }
  return sanitizeCode(code);
}

AD5242Status AD5242::setStatus(const AD5242Status status) {
  _lastStatus = status;
  return status;
}

//  -- END OF FILE --

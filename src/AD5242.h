#pragma once

//    FILE: AD5242.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.3.0
// PURPOSE: I2C digital PotentioMeter AD5242
//    DATE: 2013-10-12
//     URL: https://github.com/RobTillaart/AD5242

#include <Arduino.h>
#include <Wire.h>


#define AD5242_LIB_VERSION    (F("0.3.0"))

#define AD5242_MIDPOINT       127
#define AD5242_MAX_VALUE      255

static const uint32_t AD5242_R10K  = 10000;
static const uint32_t AD5242_R100K = 100000;
static const uint32_t AD5242_R1M   = 1000000;

enum AD5242Status : uint8_t
{
  AD5242_OK = 0,
  AD5242_ERR_I2C = 1,
  AD5242_ERR_PARAM = 2,
  AD5242_ERR_NOT_INITIALIZED = 3
};

class AD5242 {
public:
  // hwEnablePin: 0 disables HW enable, 1 uses O1, 2 uses O2.
  explicit AD5242(const uint8_t address, const uint8_t hwEnablePin = 0, TwoWire *wire = &Wire);

  AD5242Status begin(const uint32_t potRating);
  bool    isConnected() const;
  uint8_t address() const;
  uint8_t channelCount() const;
  bool    isInitialized() const;
  uint32_t potRating() const;
  AD5242Status lastStatus() const;
  uint8_t hwEnablePin() const;
  AD5242Status setABRvalue(const uint8_t rdac, const uint32_t abResistance);

  // READ WRITE
  // RDAC channels are 1 or 2 (datasheet naming).
  uint8_t getLastValue(const uint8_t rdac);
  uint32_t getResistance(const uint8_t rdac, const char direction = 'B');
  uint32_t getResistance(const uint8_t rdac, const char *direction);
  uint8_t getResistancePercent(const uint8_t rdac, const char direction = 'B');
  uint8_t getResistancePercent(const uint8_t rdac, const char *direction);
  AD5242Status write(const uint8_t rdac, const uint8_t value);
  AD5242Status write(const uint8_t rdac, const uint8_t value, const bool O1, const bool O2);
  AD5242Status writeResistance(const uint8_t rdac, const uint32_t value);
  AD5242Status writeResistance(const uint8_t rdac, const uint32_t value, const char direction);
  AD5242Status writeResistance(const uint8_t rdac, const uint32_t value, const char *direction);
  AD5242Status writeResistance(const uint8_t rdac, const uint32_t value, const bool O1, const bool O2);
  AD5242Status writeResistance(const uint8_t rdac, const uint32_t value, const bool O1, const bool O2, const char direction);
  AD5242Status writeResistance(const uint8_t rdac, const uint32_t value, const bool O1, const bool O2, const char *direction);

  // IO LINES
  AD5242Status setO1(const bool value = true);
  AD5242Status setO2(const bool value = true);
  bool getO1() const;
  bool getO2() const;
  // output is 1 or 2.
  AD5242Status setO(const uint8_t output, const bool value);
  bool getO(const uint8_t output);

  // HARDWARE ENABLE (O1 or O2)
  AD5242Status HWEnable();
  AD5242Status HWDisable();

  // experimental - to be tested - use at own risk
  AD5242Status shutDown();     // datasheet P15

  // RESET
  AD5242Status reset();    // reset both channels to AD5242_MIDPOINT and O1/O2 to LOW
  AD5242Status zeroAll();  // set both channels to 0 and O1/O2 to LOW
  AD5242Status midScaleReset(const uint8_t rdac);

  // DEBUGGING
  AD5242Status readBackRegister(uint8_t &value);

private:
  static const uint8_t kChannelCount = 2;
  static bool isValidRating(const uint32_t potRating);
  static uint8_t channelIndex(const uint8_t rdac);
  static uint8_t outputMask(const uint8_t output);
  static char normalizeDirection(const char direction);
  static char normalizeDirection(const char *direction);
  uint32_t potRatingFor(const uint8_t idx) const;
  AD5242Status setStatus(const AD5242Status status);

  AD5242Status send(const uint8_t cmd, const uint8_t value);

  uint8_t _address;
  TwoWire * _wire;
  uint8_t _lastValue[kChannelCount];
  uint8_t _O1;
  uint8_t _O2;
  uint32_t _potRating;
  uint32_t _potRatingAB[kChannelCount];
  bool _hasRating;
  uint8_t _hwEnablePin;
  AD5242Status _lastStatus;
};

//  -- END OF FILE --

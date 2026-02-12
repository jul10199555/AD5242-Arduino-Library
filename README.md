# AD5242

Arduino library for the AD5242 I2C dual digital potentiometer.

## Description

The AD5242 provides two potentiometer channels (RDAC1 and RDAC2) with 256 steps.
Available resistance ratings are 10k, 100k, and 1M.

The device powers up at midscale. This library defines `AD5242_MIDPOINT` as 127.

## I2C Address

The AD5242 has two address pins (AD1, AD0) to configure the I2C address 0x2C - 0x2F.

| Addr (Hex) | AD1 | AD0 |
|:----------:|:---:|:---:|
|   0x2C     | GND | GND |
|   0x2D     | GND | VDD |
|   0x2E     | VDD | GND |
|   0x2F     | VDD | VDD |

## Usage

```cpp
#include "AD5242.h"

AD5242 pot(0x2C);

void setup() {
  Wire.begin();
  AD5242Status status = pot.begin(AD5242_R100K);
  if (status != AD5242_OK) {
    // handle error
  }
}
```

`begin()` does not call `Wire.begin()`; you must initialize the I2C bus yourself.

## API

### Initialization and Info

- `AD5242Status begin(uint32_t potRating)` initializes the device and caches the rating.
- `bool isConnected() const` checks if the device ACKs on the I2C bus.
- `uint8_t address() const` returns the configured I2C address.
- `uint8_t channelCount() const` returns 2.
- `bool isInitialized() const` returns whether a valid rating was provided.
- `uint32_t potRating() const` returns the configured rating.
- `AD5242Status lastStatus() const` returns the status of the last API call that sets it.
- `uint8_t hwEnablePin() const` returns the configured HW enable output (1 or 2), or 0 if disabled.
- `AD5242Status setABRvalue(uint8_t rdac, uint32_t abResistance)` sets the measured A-to-B resistance for the channel.

Valid ratings:
- `AD5242_R10K`
- `AD5242_R100K`
- `AD5242_R1M`

If `setABRvalue()` is used, that per-channel value is used for resistance calculations and `writeResistance()` instead of the nominal rating.

### Read / Cached Values

The AD5242 does not support reading RDAC values directly. This library caches values written by `write()` and
returns them from the methods below.

Channel numbers are **1** and **2** (datasheet naming).

- `uint8_t getLastValue(uint8_t rdac)`
- `uint32_t getResistance(uint8_t rdac, char direction = 'B')`
- `uint32_t getResistance(uint8_t rdac, const char *direction)`
- `uint8_t getResistancePercent(uint8_t rdac, char direction = 'B')`
- `uint8_t getResistancePercent(uint8_t rdac, const char *direction)`

Direction:
- `'B'` (default) returns B-to-wiper resistance.
- `'A'` returns A-to-wiper resistance.
Both `char` and string inputs (e.g. `"A"`) are accepted.

Use `lastStatus()` to detect errors from these value-returning methods.

### Write

- `AD5242Status write(uint8_t rdac, uint8_t value)`
- `AD5242Status write(uint8_t rdac, uint8_t value, bool o1, bool o2)`
- `AD5242Status writeResistance(uint8_t rdac, uint32_t value)`
- `AD5242Status writeResistance(uint8_t rdac, uint32_t value, char direction)`
- `AD5242Status writeResistance(uint8_t rdac, uint32_t value, const char *direction)`
- `AD5242Status writeResistance(uint8_t rdac, uint32_t value, bool o1, bool o2)`
- `AD5242Status writeResistance(uint8_t rdac, uint32_t value, bool o1, bool o2, char direction)`
- `AD5242Status writeResistance(uint8_t rdac, uint32_t value, bool o1, bool o2, const char *direction)`

Direction selects whether `value` refers to A-to-wiper (`'A'`) or B-to-wiper (`'B'`, default).
`rdac` is **1** or **2**.

### Output Lines

- `AD5242Status setO1(bool value = true)`
- `AD5242Status setO2(bool value = true)`
- `bool getO1() const`
- `bool getO2() const`
- `AD5242Status setO(uint8_t output, bool value)`
- `bool getO(uint8_t output)`

`output` is **1** or **2**.

### Hardware Enable

Use one of the output lines (O1 or O2) as a hardware enable pin by passing it in the constructor:

```cpp
AD5242 pot(0x2C, 1, &Wire);  // use O1 as HW enable
```

- `AD5242Status HWEnable()` sets the configured output HIGH.
- `AD5242Status HWDisable()` sets the configured output LOW.

If `hwEnablePin` is configured, `begin()` will enable it by default.
`reset()` will also re-enable it after resetting the channels.

### Reset / Misc

- `AD5242Status reset()` sets both channels to midpoint and outputs to LOW.
- `AD5242Status zeroAll()` sets both channels to 0 and outputs to LOW.
- `AD5242Status midScaleReset(uint8_t rdac)` resets one channel to midpoint.
- `AD5242Status readBackRegister(uint8_t &value)` reads the last value written (datasheet).
- `AD5242Status shutDown()` issues the shutdown command (datasheet).

## Error Codes

| Code | Meaning |
|:----:|:--------|
| `AD5242_OK` | Success |
| `AD5242_ERR_I2C` | I2C transaction failed |
| `AD5242_ERR_PARAM` | Invalid parameter |
| `AD5242_ERR_NOT_INITIALIZED` | Rating not set |

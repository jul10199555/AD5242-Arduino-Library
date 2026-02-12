# Change Log AD5242

All notable changes to this project will be documented in this file.
The format is based on Keep a Changelog and this project adheres to Semantic Versioning.

## [0.3.0] - 2026-02-12
- breaking: switch RDAC numbering to 1/2 to match datasheet.
- breaking: simplify cached read methods to return values directly and add lastStatus().
- add direction support (A/B) for resistance reads and writes.
- add writeResistance() overload without O1/O2 parameters.
- add HWEnable/HWDisable using O1 or O2.
- add setABRvalue() for per-channel A-B calibration.
- update examples, README, and keywords.

## [0.2.0] - 2026-02-11
- breaking: refactor to AD5242-only API and documentation.
- breaking: replace ambiguous value returns with status + out-parameter API.
- add rating constants and initialization state helpers.
- fix writeResistance() to allow full-scale values.
- fix cached value updates to only occur after successful I2C writes.
- fix constructor initialization order warning.
- update examples, README, and keywords.

## [0.1.2] - 2013-10-12
- initial release.

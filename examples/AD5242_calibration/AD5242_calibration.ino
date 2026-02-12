//
//    FILE: AD5242_calibration.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: AD5242 demo program - per-channel A-B calibration
//     URL: https://github.com/RobTillaart/AD5242


#include "AD5242.h"

AD5242 AD01(0x2C);  //  AD0 & AD1 == GND

// Replace these with your measured A-B resistances (Ohms).
const uint32_t AB_R1 = 101200;  // RDAC1 A-B measured
const uint32_t AB_R2 =  98700;  // RDAC2 A-B measured


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("AD5242_LIB_VERSION: ");
  Serial.println(AD5242_LIB_VERSION);

  Wire.begin();
  Wire.setClock(400000);

  AD5242Status status = AD01.begin(AD5242_R100K);
  Serial.print("begin: ");
  Serial.println(status == AD5242_OK ? "OK" : "ERR");

  // Apply per-channel A-B calibration values.
  AD01.setABRvalue(1, AB_R1);
  AD01.setABRvalue(2, AB_R2);
}


void loop()
{
  for (int val = 0; val <= AD5242_MAX_VALUE; val += 64)
  {
    AD01.write(1, val);
    AD01.write(2, AD5242_MAX_VALUE - val);

    uint32_t r1b = AD01.getResistance(1, 'B');
    uint32_t r1a = AD01.getResistance(1, 'A');
    uint32_t r2b = AD01.getResistance(2, 'B');
    uint32_t r2a = AD01.getResistance(2, 'A');

    Serial.print("RDAC1 B/A: ");
    Serial.print(r1b);
    Serial.print(" / ");
    Serial.print(r1a);
    Serial.print("   RDAC2 B/A: ");
    Serial.print(r2b);
    Serial.print(" / ");
    Serial.println(r2a);

    delay(500);
  }
  delay(2000);
}


//  -- END OF FILE --

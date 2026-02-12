//
//    FILE: AD5242_cachedRead.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: AD5242 demo program
//     URL: https://github.com/RobTillaart/AD5242


#include "AD5242.h"

AD5242 AD01(0x2C);  //  AD0 & AD1 == GND


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
  Serial.print("isConnected: ");
  Serial.println(AD01.isConnected() ? "true" : "false");
}


void loop()
{
  for (int val = 0; val < 255; val++)
  {
    Serial.print(val);
    AD01.write(2, val);
    delay(100);

    uint8_t x = AD01.getLastValue(2);
    AD5242Status rv = AD01.lastStatus();
    Serial.print('\t');
    Serial.print(x);
    Serial.print('\t');
    Serial.println(rv);
    delay(100);
  }
  delay(1000);
}


//  -- END OF FILE --

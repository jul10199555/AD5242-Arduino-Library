//
//    FILE: AD5242_write_RP2040.ino
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

  Wire.setSDA(12);  //  adjust if needed
  Wire.setSCL(13);  //  adjust if needed
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
  for (int val = 1; val < 255; val++)
  {
    AD5242Status rv = AD01.write(2, val);
    if (val == 200)
    {
      rv = AD01.write(2, val, true, false);
    }
    if (val == 1)
    {
      rv = AD01.write(2, val, false, false);
    }
    Serial.print(val);
    Serial.print('\t');
    Serial.println(rv);
    delay(20);
  }
}


//  -- END OF FILE --

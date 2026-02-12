//
//    FILE: AD5242_midScaleReset.ino
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
  Serial.println(255);
  AD01.write(2, 255);
  delay(1000);

  Serial.println("midScaleReset(2)");
  AD01.midScaleReset(2);
  delay(1000);

  Serial.println(0);
  AD01.write(2, 0);
  delay(1000);

  Serial.println("midScaleReset(2)");
  AD01.midScaleReset(2);
  delay(1000);
}


//  -- END OF FILE --


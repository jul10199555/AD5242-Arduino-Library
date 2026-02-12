//
//    FILE: AD5242_setO.ino
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
  int x = 10 + analogRead(A0);
  AD01.setO1(false);
  AD01.setO2(true);
  Serial.print(AD01.getO1());
  Serial.print('\t');
  Serial.println(AD01.getO2());
  delay(x);

  x = 10 + analogRead(A0);
  AD01.setO1(true);
  AD01.setO2(false);
  Serial.print(AD01.getO1());
  Serial.print('\t');
  Serial.println(AD01.getO2());
  delay(x);
}


//  -- END OF FILE --


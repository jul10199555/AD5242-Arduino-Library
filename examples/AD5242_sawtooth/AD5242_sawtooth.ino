//
//    FILE: AD5242_sawtooth.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: AD5242 demo program
//     URL: https://github.com/RobTillaart/AD5242
//
//  to be viewed with the IDE plotter


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
  for (int val = 0; val < 256; val++)
  {
    //  two "inverted" sawtooths
    AD01.write(1, 255 - val);
    AD01.write(2, val);
    Serial.print(255 - val);
    Serial.print("\t");
    Serial.println(val);
    delay(20);
  }
}


//  -- END OF FILE --

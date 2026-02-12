//
//    FILE: AD5242_readValues.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: AD5242 demo program - cached values and computed resistance
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
  bool o1 = false;
  bool o2 = true;

  for (int val = 0; val <= AD5242_MAX_VALUE; val += 51)
  {
    o1 = !o1;
    o2 = !o2;
    AD01.setO(1, o1);
    AD01.setO(2, o2);

    AD5242Status w0 = AD01.write(1, val, o1, o2);
    AD5242Status w1 = AD01.write(2, AD5242_MAX_VALUE - val, o1, o2);

    uint8_t last0 = AD01.getLastValue(1);
    AD5242Status lv0 = AD01.lastStatus();
    uint8_t last1 = AD01.getLastValue(2);
    AD5242Status lv1 = AD01.lastStatus();

    uint32_t r0b = AD01.getResistance(1, 'B');
    AD5242Status r0bS = AD01.lastStatus();
    uint32_t r0a = AD01.getResistance(1, "A");
    AD5242Status r0aS = AD01.lastStatus();
    uint32_t r1b = AD01.getResistance(2, 'B');
    AD5242Status r1bS = AD01.lastStatus();
    uint32_t r1a = AD01.getResistance(2, "A");
    AD5242Status r1aS = AD01.lastStatus();

    uint8_t p0b = AD01.getResistancePercent(1, 'B');
    AD5242Status p0bS = AD01.lastStatus();
    uint8_t p0a = AD01.getResistancePercent(1, "A");
    AD5242Status p0aS = AD01.lastStatus();
    uint8_t p1b = AD01.getResistancePercent(2, 'B');
    AD5242Status p1bS = AD01.lastStatus();
    uint8_t p1a = AD01.getResistancePercent(2, "A");
    AD5242Status p1aS = AD01.lastStatus();

    Serial.print("O1=");
    Serial.print(AD01.getO(1));
    Serial.print("\tO2=");
    Serial.print(AD01.getO(2));
    Serial.print("\tset=");
    Serial.print(val);
    Serial.print(",");
    Serial.print(AD5242_MAX_VALUE - val);
    Serial.print("\twrite=");
    Serial.print(w0);
    Serial.print(",");
    Serial.print(w1);
    Serial.print("\tlast=");
    Serial.print(last0);
    Serial.print(",");
    Serial.print(last1);
    Serial.print("\tR(B/A)=");
    Serial.print(r0b);
    Serial.print("/");
    Serial.print(r0a);
    Serial.print(",");
    Serial.print(r1b);
    Serial.print("/");
    Serial.print(r1a);
    Serial.print("\t%(B/A)=");
    Serial.print(p0b);
    Serial.print("/");
    Serial.print(p0a);
    Serial.print(",");
    Serial.print(p1b);
    Serial.print("/");
    Serial.print(p1a);
    Serial.print("\tstatus=");
    Serial.print(lv0);
    Serial.print(",");
    Serial.print(lv1);
    Serial.print(",");
    Serial.print(r0bS);
    Serial.print(",");
    Serial.print(r0aS);
    Serial.print(",");
    Serial.print(r1bS);
    Serial.print(",");
    Serial.print(r1aS);
    Serial.print(",");
    Serial.print(p0bS);
    Serial.print(",");
    Serial.print(p0aS);
    Serial.print(",");
    Serial.print(p1bS);
    Serial.print(",");
    Serial.println(p1aS);

    delay(500);
  }
  delay(2000);
}


//  -- END OF FILE --

/*
  I2C scanner for Fingertip wiring debug.

  Expected for one PCA9685 board:
    Found I2C device at 0x40

  Wiring:
    Nano A4  -> PCA9685 SDA
    Nano A5  -> PCA9685 SCL
    Nano 5V  -> PCA9685 VCC
    Nano GND -> PCA9685 GND
*/

#include <Wire.h>

void setup() {
  Serial.begin(9600);
  delay(1500);
  Serial.println("I2C_SCANNER_READY");
  Wire.begin();
  Wire.setWireTimeout(25000, true);
}

void loop() {
  byte count = 0;

  Serial.println("Scanning...");
  Serial.flush();

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Found I2C device at 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      count++;
    }
  }

  if (count == 0) {
    Serial.println("No I2C devices found");
  }

  delay(2000);
}

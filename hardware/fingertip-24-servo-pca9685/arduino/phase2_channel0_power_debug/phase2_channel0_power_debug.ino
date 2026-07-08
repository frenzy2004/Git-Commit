/*
  Temporary debug sketch.

  It continuously moves only PCA9685 channel 0 between two positions.
  Use ONE servo on channel 0 while checking servo power wiring.
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

int pulseForAngle(int angle) {
  return map(angle, 0, 90, 150, 400);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(50);
  delay(100);
  Serial.println("CHANNEL0_DEBUG_RUNNING");
}

void loop() {
  Serial.println("CH0 0");
  pwm.setPWM(0, 0, pulseForAngle(0));
  delay(1000);

  Serial.println("CH0 90");
  pwm.setPWM(0, 0, pulseForAngle(90));
  delay(1000);
}

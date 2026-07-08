/*
  Obvious servo-power test.

  Uses PCA9685 board at 0x40, channel 0 only.
  It moves channel 0 to 0 degrees, waits, then moves to 90 degrees and holds.
  If this does not move, the servo rail V+ / GND / plug orientation is the issue.
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
  delay(300);

  Serial.println("BOARD_0X40_CH0_TO_0");
  pwm.setPWM(0, 0, pulseForAngle(0));
  delay(3000);

  Serial.println("BOARD_0X40_CH0_TO_90_HOLD");
  pwm.setPWM(0, 0, pulseForAngle(90));
}

void loop() {
}

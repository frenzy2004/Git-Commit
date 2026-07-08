/*
  Continuous 24-servo test for Fingertip.

  Board 1: PCA9685 at 0x40, channels 0-11 used.
  Board 2: PCA9685 at 0x41, channels 0-11 used.

  This sketch moves one servo at a time and prints the logical channel:
    T:0  through T:23

  Mapping:
    T:0  - T:11 -> board 0x40 channels 0-11
    T:12 - T:23 -> board 0x41 channels 0-11
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver board2 = Adafruit_PWMServoDriver(0x41);

const int SERVO_FREQ_HZ = 50;
const int PULSE_0_DEG = 150;
const int PULSE_90_DEG = 400;
const int HOLD_MS = 650;
const int BETWEEN_MS = 250;

void setLogicalChannel(int logicalChannel, int pulse) {
  if (logicalChannel < 12) {
    board1.setPWM(logicalChannel, 0, pulse);
  } else {
    board2.setPWM(logicalChannel - 12, 0, pulse);
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  board1.begin();
  board2.begin();
  board1.setPWMFreq(SERVO_FREQ_HZ);
  board2.setPWMFreq(SERVO_FREQ_HZ);
  delay(300);

  Serial.println("CONTINUOUS_24_SERVO_TEST_READY");
  Serial.println("One servo moves at a time: T:0 through T:23");

  for (int channel = 0; channel < 24; channel++) {
    setLogicalChannel(channel, PULSE_0_DEG);
    delay(30);
  }
}

void loop() {
  for (int channel = 0; channel < 24; channel++) {
    Serial.print("T:");
    Serial.println(channel);

    setLogicalChannel(channel, PULSE_90_DEG);
    delay(HOLD_MS);
    setLogicalChannel(channel, PULSE_0_DEG);
    delay(BETWEEN_MS);
  }

  Serial.println("CYCLE_DONE");
}

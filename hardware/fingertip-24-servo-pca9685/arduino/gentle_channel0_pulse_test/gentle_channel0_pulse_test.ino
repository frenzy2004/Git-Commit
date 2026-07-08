/*
  Gentle PCA9685 channel 0 test for SG90 troubleshooting.

  Uses board 0x40, channel 0 only.
  Avoids extreme servo endpoints:
    180 -> 300 -> 420 -> 300, repeat
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

const int SERVO_FREQ_HZ = 50;
const int PULSE_LOW = 180;
const int PULSE_MID = 300;
const int PULSE_HIGH = 420;
const int STEP_DELAY_MS = 8;
const int HOLD_MS = 600;

void movePulseSmooth(int fromPulse, int toPulse) {
  int step = (toPulse >= fromPulse) ? 1 : -1;

  for (int pulse = fromPulse; pulse != toPulse; pulse += step) {
    pwm.setPWM(0, 0, pulse);
    delay(STEP_DELAY_MS);
  }

  pwm.setPWM(0, 0, toPulse);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ_HZ);
  delay(300);

  Serial.println("GENTLE_CH0_TEST_READY");
  pwm.setPWM(0, 0, PULSE_MID);
  delay(1000);
}

void loop() {
  Serial.println("CH0 MID_TO_LOW");
  movePulseSmooth(PULSE_MID, PULSE_LOW);
  delay(HOLD_MS);

  Serial.println("CH0 LOW_TO_MID");
  movePulseSmooth(PULSE_LOW, PULSE_MID);
  delay(HOLD_MS);

  Serial.println("CH0 MID_TO_HIGH");
  movePulseSmooth(PULSE_MID, PULSE_HIGH);
  delay(HOLD_MS);

  Serial.println("CH0 HIGH_TO_MID");
  movePulseSmooth(PULSE_HIGH, PULSE_MID);
  delay(HOLD_MS);
}

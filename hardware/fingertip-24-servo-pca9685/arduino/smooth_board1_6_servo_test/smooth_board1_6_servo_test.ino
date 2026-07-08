/*
  Smooth board-1-only PCA9685 test.

  Uses only the first driver board at 0x40.
  Moves channels 0-5 slowly, one at a time.

  This matches the earlier "fantastic" movement style:
    90 -> 0 slowly
    0 -> 90 slowly
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);

const int PULSE_0_DEG = 150;
const int PULSE_90_DEG = 400;
const int STEP_DELAY_MS = 25;
const int HOLD_MS = 350;

int pulseForAngle(int angle) {
  angle = constrain(angle, 0, 90);
  return map(angle, 0, 90, PULSE_0_DEG, PULSE_90_DEG);
}

void writeAngle(int channel, int angle) {
  board1.setPWM(channel, 0, pulseForAngle(angle));
}

void moveSmooth(int channel, int fromAngle, int toAngle) {
  int step = (toAngle >= fromAngle) ? 1 : -1;

  for (int angle = fromAngle; angle != toAngle; angle += step) {
    writeAngle(channel, angle);
    delay(STEP_DELAY_MS);
  }

  writeAngle(channel, toAngle);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  board1.begin();
  board1.setPWMFreq(50);
  delay(300);

  Serial.println("SMOOTH_BOARD1_6_SERVO_TEST_READY");

  for (int channel = 0; channel < 6; channel++) {
    writeAngle(channel, 90);
    delay(40);
  }
}

void loop() {
  for (int channel = 0; channel < 6; channel++) {
    Serial.print("BOARD1 CHANNEL ");
    Serial.print(channel);
    Serial.println(" 90_TO_0");
    moveSmooth(channel, 90, 0);
    delay(HOLD_MS);

    Serial.print("BOARD1 CHANNEL ");
    Serial.print(channel);
    Serial.println(" 0_TO_90");
    moveSmooth(channel, 0, 90);
    delay(HOLD_MS);
  }

  Serial.println("BOARD1_CYCLE_DONE");
}

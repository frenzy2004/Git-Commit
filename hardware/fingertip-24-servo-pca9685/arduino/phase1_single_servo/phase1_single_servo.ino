/*
  Fingertip Phase 1: one SG90 servo on Arduino Nano pin D9.

  Wiring:
    Servo orange/yellow signal -> Nano D9
    Servo red                 -> 4xAA battery pack +
    Servo brown/black GND      -> 4xAA battery pack - AND Nano GND

  Serial Monitor:
    Baud: 9600
    Send: w  -> raise pin
          r  -> retract pin
*/

#include <Servo.h>

const byte SERVO_PIN = 9;
const int DOWN_ANGLE = 90;
const int UP_ANGLE = 0;
const int STEP_DELAY_MS = 25;

Servo pinServo;
int currentAngle = DOWN_ANGLE;

void moveServoSmooth(int targetAngle) {
  int step = (targetAngle >= currentAngle) ? 1 : -1;

  while (currentAngle != targetAngle) {
    currentAngle += step;
    pinServo.write(currentAngle);
    delay(STEP_DELAY_MS);
  }
}

void setup() {
  Serial.begin(9600);
  pinServo.attach(SERVO_PIN);
  pinServo.write(DOWN_ANGLE);
  currentAngle = DOWN_ANGLE;

  Serial.println("FINGERTIP_PHASE1_READY");
  Serial.println("Send w to raise, r to retract.");
}

void loop() {
  if (!Serial.available()) {
    return;
  }

  char command = Serial.read();

  if (command == 'w' || command == 'W') {
    moveServoSmooth(UP_ANGLE);
    Serial.println("UP");
  } else if (command == 'r' || command == 'R') {
    moveServoSmooth(DOWN_ANGLE);
    Serial.println("DOWN");
  }
}

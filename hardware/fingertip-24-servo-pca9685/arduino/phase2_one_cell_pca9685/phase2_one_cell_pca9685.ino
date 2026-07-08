/*
  Fingertip Phase 2: one braille cell on one PCA9685 board.

  Wiring:
    Nano A4  -> PCA9685 SDA
    Nano A5  -> PCA9685 SCL
    Nano 5V  -> PCA9685 VCC
    Nano GND -> PCA9685 GND

    External 5V + -> PCA9685 V+ screw terminal
    External 5V - -> PCA9685 GND screw terminal

    Servo channels:
      Channel 0 = dot 1
      Channel 1 = dot 2
      Channel 2 = dot 3
      Channel 3 = dot 4
      Channel 4 = dot 5
      Channel 5 = dot 6

  Serial Monitor:
    9600 baud, newline ending
    T:0 through T:5 -> test one servo
    R               -> lower all dots
    L:A             -> show braille A
    L:Q             -> show braille Q

  Calibration:
    Your Phase 1 mechanism raised when moving from 90 to 0, so this sketch uses:
      DOWN_ANGLE = 90
      UP_ANGLE   = 0
    If the pins move backward, swap those two values.
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

const int SERVO_FREQ_HZ = 50;
const int SERVO_MIN_PULSE = 150;
const int SERVO_90_PULSE = 400;
const int DOWN_ANGLE = 90;
const int UP_ANGLE = 0;
const int WRITE_GAP_MS = 50;
const int TEST_HOLD_MS = 500;

const bool BRAILLE[26][6] = {
  {1, 0, 0, 0, 0, 0},  // A
  {1, 1, 0, 0, 0, 0},  // B
  {1, 0, 0, 1, 0, 0},  // C
  {1, 0, 0, 1, 1, 0},  // D
  {1, 0, 0, 0, 1, 0},  // E
  {1, 1, 0, 1, 0, 0},  // F
  {1, 1, 0, 1, 1, 0},  // G
  {1, 1, 0, 0, 1, 0},  // H
  {0, 1, 0, 1, 0, 0},  // I
  {0, 1, 0, 1, 1, 0},  // J
  {1, 0, 1, 0, 0, 0},  // K
  {1, 1, 1, 0, 0, 0},  // L
  {1, 0, 1, 1, 0, 0},  // M
  {1, 0, 1, 1, 1, 0},  // N
  {1, 0, 1, 0, 1, 0},  // O
  {1, 1, 1, 1, 0, 0},  // P
  {1, 1, 1, 1, 1, 0},  // Q
  {1, 1, 1, 0, 1, 0},  // R
  {0, 1, 1, 1, 0, 0},  // S
  {0, 1, 1, 1, 1, 0},  // T
  {1, 0, 1, 0, 0, 1},  // U
  {1, 1, 1, 0, 0, 1},  // V
  {0, 1, 0, 1, 1, 1},  // W
  {1, 0, 1, 1, 0, 1},  // X
  {1, 0, 1, 1, 1, 1},  // Y
  {1, 0, 1, 0, 1, 1},  // Z
};

String inputLine = "";

int pulseForAngle(int angle) {
  angle = constrain(angle, 0, 90);
  return map(angle, 0, 90, SERVO_MIN_PULSE, SERVO_90_PULSE);
}

void setChannel(int channel, bool up) {
  int angle = up ? UP_ANGLE : DOWN_ANGLE;
  pwm.setPWM(channel, 0, pulseForAngle(angle));
  delay(WRITE_GAP_MS);
}

void clearAll() {
  for (int channel = 0; channel < 6; channel++) {
    setChannel(channel, false);
  }
  Serial.println("CLEAR");
}

void testChannel(int channel) {
  if (channel < 0 || channel > 5) {
    Serial.println("ERR TEST_RANGE_0_5");
    return;
  }

  setChannel(channel, false);
  delay(TEST_HOLD_MS);
  setChannel(channel, true);
  delay(TEST_HOLD_MS);
  setChannel(channel, false);
  Serial.print("TESTED ");
  Serial.println(channel);
}

void showLetter(char letter) {
  letter = toupper(letter);

  if (letter < 'A' || letter > 'Z') {
    Serial.println("ERR LETTER_A_Z");
    return;
  }

  int index = letter - 'A';
  for (int dot = 0; dot < 6; dot++) {
    setChannel(dot, BRAILLE[index][dot]);
  }

  Serial.print("SHOW ");
  Serial.println(letter);
}

void handleCommand(String command) {
  command.trim();
  command.toUpperCase();

  if (command.length() == 0) {
    return;
  }

  if (command == "R") {
    clearAll();
    return;
  }

  if (command.startsWith("T:")) {
    testChannel(command.substring(2).toInt());
    return;
  }

  if (command.startsWith("L:") && command.length() >= 3) {
    showLetter(command.charAt(2));
    return;
  }

  Serial.println("ERR BAD_COMMAND");
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ_HZ);
  delay(10);

  clearAll();
  Serial.println("READY");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      handleCommand(inputLine);
      inputLine = "";
    } else {
      inputLine += c;
    }
  }
}

/*
  Fingertip main firmware: 4 braille cells, 24 SG90 servos.

  Hardware:
    Arduino Nano
    PCA9685 #1 at address 0x40
    PCA9685 #2 at address 0x60, based on the live I2C scan

  Wiring:
    Nano A4 SDA -> both PCA9685 SDA
    Nano A5 SCL -> both PCA9685 SCL
    Nano 5V     -> both PCA9685 VCC
    Nano GND    -> both PCA9685 GND
    5V 5A +     -> both PCA9685 V+ screw terminals
    5V 5A -     -> both PCA9685 GND screw terminals

  Channel map from the build guide images:
    Cell 1 dots 1-6 -> board 0x40 channels 0-5
    Cell 2 dots 1-6 -> board 0x40 channels 6-11
    Cell 3 dots 1-6 -> board 0x41 channels 0-5
    Cell 4 dots 1-6 -> board 0x41 channels 6-11

  Serial commands, 9600 baud, newline optional:
    R          lower all pins
    L:A        show letter A on cell 1
    L:2:A      show letter A on cell 2
    W:CUP      show up to 4 letters across cells 1-4
    T:5        test global servo 5
    T:3:2      test cell 3 dot 2
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

Adafruit_PWMServoDriver pwm0 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x60);

const int SERVO_FREQ_HZ = 50;
const int SERVO_DOWN_ANGLE = 0;
const int SERVO_UP_ANGLE = 60;
const int SERVO_MIN_PULSE = 150;  // about 0 degrees at 50 Hz
const int SERVO_90_PULSE = 400;   // about 90 degrees at 50 Hz
const int WRITE_GAP_MS = 50;
const int TEST_HOLD_MS = 450;

struct ServoRef {
  byte board;
  byte channel;
};

const ServoRef CELL_DOTS[4][6] = {
  {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}},
  {{0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10}, {0, 11}},
  {{1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}},
  {{1, 6}, {1, 7}, {1, 8}, {1, 9}, {1, 10}, {1, 11}},
};

const byte BRAILLE[26][6] = {
  {1, 0, 0, 0, 0, 0},  // A: 1
  {1, 1, 0, 0, 0, 0},  // B: 1,2
  {1, 0, 0, 1, 0, 0},  // C: 1,4
  {1, 0, 0, 1, 1, 0},  // D: 1,4,5
  {1, 0, 0, 0, 1, 0},  // E: 1,5
  {1, 1, 0, 1, 0, 0},  // F: 1,2,4
  {1, 1, 0, 1, 1, 0},  // G: 1,2,4,5
  {1, 1, 0, 0, 1, 0},  // H: 1,2,5
  {0, 1, 0, 1, 0, 0},  // I: 2,4
  {0, 1, 0, 1, 1, 0},  // J: 2,4,5
  {1, 0, 1, 0, 0, 0},  // K: 1,3
  {1, 1, 1, 0, 0, 0},  // L: 1,2,3
  {1, 0, 1, 1, 0, 0},  // M: 1,3,4
  {1, 0, 1, 1, 1, 0},  // N: 1,3,4,5
  {1, 0, 1, 0, 1, 0},  // O: 1,3,5
  {1, 1, 1, 1, 0, 0},  // P: 1,2,3,4
  {1, 1, 1, 1, 1, 0},  // Q: 1,2,3,4,5
  {1, 1, 1, 0, 1, 0},  // R: 1,2,3,5
  {0, 1, 1, 1, 0, 0},  // S: 2,3,4
  {0, 1, 1, 1, 1, 0},  // T: 2,3,4,5
  {1, 0, 1, 0, 0, 1},  // U: 1,3,6
  {1, 1, 1, 0, 0, 1},  // V: 1,2,3,6
  {0, 1, 0, 1, 1, 1},  // W: 2,4,5,6
  {1, 0, 1, 1, 0, 1},  // X: 1,3,4,6
  {1, 0, 1, 1, 1, 1},  // Y: 1,3,4,5,6
  {1, 0, 1, 0, 1, 1},  // Z: 1,3,5,6
};

char lineBuffer[48];
byte lineLength = 0;

int angleToPulse(int angle) {
  angle = constrain(angle, 0, 90);
  return map(angle, 0, 90, SERVO_MIN_PULSE, SERVO_90_PULSE);
}

void setPwm(byte board, byte channel, int pulse) {
  if (board == 0) {
    pwm0.setPWM(channel, 0, pulse);
  } else {
    pwm1.setPWM(channel, 0, pulse);
  }
}

void writeServo(ServoRef servo, int angle) {
  setPwm(servo.board, servo.channel, angleToPulse(angle));
}

void setCellBlank(byte cellIndex) {
  for (byte dot = 0; dot < 6; dot++) {
    writeServo(CELL_DOTS[cellIndex][dot], SERVO_DOWN_ANGLE);
    delay(WRITE_GAP_MS);
  }
}

void lowerAll() {
  for (byte cell = 0; cell < 4; cell++) {
    setCellBlank(cell);
  }
  Serial.println("DOWN");
}

bool isLetter(char letter) {
  letter = toupper(letter);
  return letter >= 'A' && letter <= 'Z';
}

void showLetter(byte cellIndex, char letter) {
  letter = toupper(letter);

  if (cellIndex >= 4) {
    Serial.println("ERR BAD_CELL");
    return;
  }

  if (!isLetter(letter)) {
    setCellBlank(cellIndex);
    Serial.println("BLANK");
    return;
  }

  byte letterIndex = letter - 'A';

  for (byte dot = 0; dot < 6; dot++) {
    int angle = BRAILLE[letterIndex][dot] ? SERVO_UP_ANGLE : SERVO_DOWN_ANGLE;
    writeServo(CELL_DOTS[cellIndex][dot], angle);
    delay(WRITE_GAP_MS);
  }

  Serial.print("CELL ");
  Serial.print(cellIndex + 1);
  Serial.print(" LETTER ");
  Serial.println(letter);
}

void showWord(const char *word) {
  for (byte cell = 0; cell < 4; cell++) {
    char letter = word[cell];
    if (letter == '\0') {
      setCellBlank(cell);
    } else {
      showLetter(cell, letter);
    }
  }
  Serial.println("WORD_DONE");
}

ServoRef globalChannelToServo(byte globalChannel) {
  if (globalChannel < 12) {
    return ServoRef{0, globalChannel};
  }

  return ServoRef{1, byte(globalChannel - 12)};
}

void testServo(ServoRef servo) {
  writeServo(servo, SERVO_DOWN_ANGLE);
  delay(TEST_HOLD_MS);
  writeServo(servo, SERVO_UP_ANGLE);
  delay(TEST_HOLD_MS);
  writeServo(servo, SERVO_DOWN_ANGLE);
  delay(TEST_HOLD_MS);
  Serial.println("TEST_DONE");
}

void handleTestCommand(const char *argument) {
  char *colon = strchr(argument, ':');

  if (colon != NULL) {
    int cell = argument[0] - '1';
    int dot = colon[1] - '1';

    if (cell < 0 || cell > 3 || dot < 0 || dot > 5) {
      Serial.println("ERR TEST_USE_T_CELL_DOT");
      return;
    }

    testServo(CELL_DOTS[cell][dot]);
    return;
  }

  int globalChannel = atoi(argument);
  if (globalChannel < 0 || globalChannel > 23) {
    Serial.println("ERR TEST_RANGE_0_23");
    return;
  }

  testServo(globalChannelToServo(byte(globalChannel)));
}

void handleLetterCommand(const char *argument) {
  if (argument[0] >= '1' && argument[0] <= '4' && argument[1] == ':') {
    showLetter(byte(argument[0] - '1'), argument[2]);
    return;
  }

  showLetter(0, argument[0]);
}

void handleCommand(char *command) {
  if (command[0] == '\0') {
    return;
  }

  command[0] = toupper(command[0]);

  if (strcmp(command, "R") == 0) {
    lowerAll();
    return;
  }

  if (strcmp(command, "S") == 0) {
    Serial.println("FINGERTIP_READY");
    return;
  }

  if (command[1] != ':') {
    Serial.println("ERR BAD_COMMAND");
    return;
  }

  if (command[0] == 'L') {
    handleLetterCommand(command + 2);
  } else if (command[0] == 'W') {
    showWord(command + 2);
  } else if (command[0] == 'T') {
    handleTestCommand(command + 2);
  } else {
    Serial.println("ERR BAD_COMMAND");
  }
}

void readSerial() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (lineLength > 0) {
        lineBuffer[lineLength] = '\0';
        handleCommand(lineBuffer);
        lineLength = 0;
      }
      continue;
    }

    if (c == ' ' || c == '\t') {
      continue;
    }

    if (lineLength < sizeof(lineBuffer) - 1) {
      lineBuffer[lineLength++] = c;
    } else {
      lineLength = 0;
      Serial.println("ERR LINE_TOO_LONG");
    }
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pwm0.begin();
  pwm1.begin();
  pwm0.setPWMFreq(SERVO_FREQ_HZ);
  pwm1.setPWMFreq(SERVO_FREQ_HZ);
  delay(10);

  lowerAll();
  Serial.println("FINGERTIP_READY");
  Serial.println("Commands: R, L:A, L:2:A, W:CUP, T:5, T:3:2");
}

void loop() {
  readSerial();
}

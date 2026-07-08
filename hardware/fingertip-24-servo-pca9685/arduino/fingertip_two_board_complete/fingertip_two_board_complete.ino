/*
  Fingertip two-board PCA9685 test firmware.

  Hardware:
    Arduino Nano over USB
    PCA9685 board 1 at 0x40
    PCA9685 board 2 at 0x41, with A0 bridged on the driver board

  Serial Monitor:
    9600 baud
    newline ending

  Commands:
    S       print status / I2C detect
    R       reset all used channels to middle pulse
    Z       reset all used channels to 0-degree pulse
    T:0     test global servo 0  (board 1, channel 0)
    T:12    test global servo 12 (board 2, channel 0)
    A       sweep all 24 one at a time
    L:1:A   show letter A on cell 1
    L:2:Q   show letter Q on cell 2
    L:3:A   show letter A on cell 3
    L:4:Q   show letter Q on cell 4
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver board2 = Adafruit_PWMServoDriver(0x41);

const byte BOARD1_ADDR = 0x40;
const byte BOARD2_ADDR = 0x41;
const int SERVO_FREQ_HZ = 50;

// Conservative SG90 pulse range. If direction is backwards, swap LOW/HIGH.
const int PULSE_LOW = 180;
const int PULSE_MID = 300;
const int PULSE_HIGH = 420;

const int STEP_DELAY_MS = 6;
const int HOLD_MS = 350;

const byte BRAILLE[26][6] = {
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
  {1, 0, 1, 0, 1, 1}   // Z
};

char line[32];
byte lineLen = 0;

Adafruit_PWMServoDriver *driverForGlobalChannel(byte globalChannel) {
  return globalChannel < 12 ? &board1 : &board2;
}

byte localChannel(byte globalChannel) {
  return globalChannel < 12 ? globalChannel : globalChannel - 12;
}

byte globalForCellDot(byte cell, byte dot) {
  if (cell < 2) {
    return cell * 6 + dot;
  }
  return 12 + (cell - 2) * 6 + dot;
}

bool i2cFound(byte address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void printI2CStatus() {
  Serial.print("I2C 0x40 board1: ");
  Serial.println(i2cFound(BOARD1_ADDR) ? "FOUND" : "MISSING");
  Serial.print("I2C 0x41 board2: ");
  Serial.println(i2cFound(BOARD2_ADDR) ? "FOUND" : "MISSING");
  Serial.print("I2C 0x70 all-call: ");
  Serial.println(i2cFound(0x70) ? "FOUND" : "MISSING");
}

void setPulse(byte globalChannel, int pulse) {
  Adafruit_PWMServoDriver *driver = driverForGlobalChannel(globalChannel);
  driver->setPWM(localChannel(globalChannel), 0, pulse);
}

void smoothTo(byte globalChannel, int fromPulse, int toPulse) {
  if (fromPulse <= toPulse) {
    for (int pulse = fromPulse; pulse <= toPulse; pulse += 3) {
      setPulse(globalChannel, pulse);
      delay(STEP_DELAY_MS);
    }
  } else {
    for (int pulse = fromPulse; pulse >= toPulse; pulse -= 3) {
      setPulse(globalChannel, pulse);
      delay(STEP_DELAY_MS);
    }
  }
  setPulse(globalChannel, toPulse);
}

void resetAll() {
  for (byte channel = 0; channel < 24; channel++) {
    setPulse(channel, PULSE_MID);
    delay(20);
  }
  Serial.println("RESET_ALL_MID");
}

void zeroAll() {
  for (byte channel = 0; channel < 24; channel++) {
    setPulse(channel, PULSE_LOW);
    delay(20);
  }
  Serial.println("RESET_ALL_0_DEG");
}

void testOne(byte globalChannel) {
  if (globalChannel > 23) {
    Serial.println("ERR T RANGE 0-23");
    return;
  }

  Serial.print("TEST global=");
  Serial.print(globalChannel);
  Serial.print(" board=");
  Serial.print(globalChannel < 12 ? "0x40" : "0x41");
  Serial.print(" channel=");
  Serial.println(localChannel(globalChannel));

  setPulse(globalChannel, PULSE_MID);
  delay(HOLD_MS);
  smoothTo(globalChannel, PULSE_MID, PULSE_LOW);
  delay(HOLD_MS);
  smoothTo(globalChannel, PULSE_LOW, PULSE_HIGH);
  delay(HOLD_MS);
  smoothTo(globalChannel, PULSE_HIGH, PULSE_MID);
  delay(HOLD_MS);

  Serial.println("TEST_DONE");
}

void testAllOneAtATime() {
  Serial.println("TEST_ALL_START");
  for (byte channel = 0; channel < 24; channel++) {
    testOne(channel);
    delay(200);
  }
  Serial.println("TEST_ALL_DONE");
}

void showLetter(byte cellNumber, char letter) {
  if (cellNumber < 1 || cellNumber > 4) {
    Serial.println("ERR CELL 1-4");
    return;
  }

  letter = toupper(letter);
  if (letter < 'A' || letter > 'Z') {
    Serial.println("ERR LETTER A-Z");
    return;
  }

  byte cell = cellNumber - 1;
  byte letterIndex = letter - 'A';

  Serial.print("SHOW cell=");
  Serial.print(cellNumber);
  Serial.print(" letter=");
  Serial.println(letter);

  for (byte dot = 0; dot < 6; dot++) {
    byte channel = globalForCellDot(cell, dot);
    setPulse(channel, BRAILLE[letterIndex][dot] ? PULSE_LOW : PULSE_HIGH);
    delay(60);
  }
  Serial.println("SHOW_DONE");
}

void handleCommand(char *cmd) {
  for (byte i = 0; cmd[i] != '\0'; i++) {
    cmd[i] = toupper(cmd[i]);
  }

  if (strcmp(cmd, "S") == 0) {
    printI2CStatus();
    Serial.println("READY");
    return;
  }

  if (strcmp(cmd, "R") == 0) {
    resetAll();
    return;
  }

  if (strcmp(cmd, "Z") == 0) {
    zeroAll();
    return;
  }

  if (strcmp(cmd, "A") == 0) {
    testAllOneAtATime();
    return;
  }

  if (cmd[0] == 'T' && cmd[1] == ':') {
    testOne((byte)atoi(cmd + 2));
    return;
  }

  if (cmd[0] == 'L' && cmd[1] == ':' && cmd[3] == ':' && cmd[4] != '\0') {
    showLetter((byte)(cmd[2] - '0'), cmd[4]);
    return;
  }

  Serial.println("ERR COMMANDS: S R Z A T:0..T:23 L:1:A");
}

void readSerial() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (lineLen > 0) {
        line[lineLen] = '\0';
        handleCommand(line);
        lineLen = 0;
      }
      continue;
    }

    if (c == ' ' || c == '\t') {
      continue;
    }

    if (lineLen < sizeof(line) - 1) {
      line[lineLen++] = c;
    } else {
      lineLen = 0;
      Serial.println("ERR LINE_TOO_LONG");
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(1500);

  Wire.begin();
  Wire.setWireTimeout(25000, true);

  board1.begin();
  board2.begin();
  board1.setPWMFreq(SERVO_FREQ_HZ);
  board2.setPWMFreq(SERVO_FREQ_HZ);
  delay(20);

  Serial.println("FINGERTIP_TWO_BOARD_READY");
  printI2CStatus();
  resetAll();
  Serial.println("Commands: S R Z A T:0 T:12 L:1:A L:3:A");
}

void loop() {
  readSerial();
}

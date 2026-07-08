/*
  Fingertip two-board calibration firmware.

  This sketch does not move any servo in setup().
  It only moves the selected/global channel when a serial command is sent.

  Serial Monitor:
    9600 baud
    newline ending

  Commands:
    S          print I2C status
    C:0        select global channel 0
    C:23       select global channel 23
    P:300      set selected channel to pulse 300
    X:5:320    set global channel 5 directly to pulse 320
    O1         cell 1 opposite-direction demo, slow and limited
    RHY        safe rhythm on T0-T23: 3s run, 3s pause, 3s run, 1s pause
    +          selected channel pulse +5
    -          selected channel pulse -5
    ?          print selected channel and pulse

  Global channel map:
    Cell 1:  0-5    board 0x40 channels 0-5
    Cell 2:  6-11   board 0x40 channels 6-11
    Cell 3:  12-17  board 0x41 channels 0-5
    Cell 4:  18-23  board 0x41 channels 6-11
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <stdlib.h>
#include <string.h>

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver board2 = Adafruit_PWMServoDriver(0x41);

const byte BOARD1_ADDR = 0x40;
const byte BOARD2_ADDR = 0x41;
const int SERVO_FREQ_HZ = 50;
const int MIN_SAFE_PULSE = 120;
const int MAX_SAFE_PULSE = 520;
const int NUDGE = 5;
const int CELL1_BASE_PULSE = 420;
const int CELL1_DEMO_AMOUNT = 20;
const int CELL1_DEMO_STEP = 5;
const int CELL1_DEMO_DELAY_MS = 450;
const byte RHYTHM_CHANNEL_COUNT = 24;
const int RHYTHM_AMOUNT = 10;
const int RHYTHM_ON_MS = 90;
const int RHYTHM_OFF_MS = 60;
const unsigned long RHYTHM_RUN_MS = 3000;
const unsigned long RHYTHM_LONG_PAUSE_MS = 3000;
const unsigned long RHYTHM_SHORT_PAUSE_MS = 1000;

const int CELL1_DIRECTION[6] = {
  -1,  // T0 baseline direction
   1,  // T1 opposite mounted
   1,  // T2 opposite mounted
   1,  // T3 opposite mounted
   1,  // T4 opposite mounted
   1   // T5 opposite mounted
};

const int RHYTHM_BASE[RHYTHM_CHANNEL_COUNT] = {
  420, 420, 420, 420, 420, 420,
  300, 300, 300, 320, 300, 300,
  300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300
};

const int RHYTHM_DIRECTION[RHYTHM_CHANNEL_COUNT] = {
  -1, 1, 1, 1, 1, 1,
   1, 1, -1, 1, 1, 1,
   1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1
};

byte selectedChannel = 0;
int selectedPulse = 300;

char line[32];
byte lineLen = 0;

Adafruit_PWMServoDriver *driverForGlobalChannel(byte globalChannel) {
  return globalChannel < 12 ? &board1 : &board2;
}

byte localChannel(byte globalChannel) {
  return globalChannel < 12 ? globalChannel : globalChannel - 12;
}

byte cellForGlobalChannel(byte globalChannel) {
  return globalChannel / 6 + 1;
}

byte dotForGlobalChannel(byte globalChannel) {
  return globalChannel % 6 + 1;
}

bool i2cFound(byte address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void printChannel(byte globalChannel) {
  Serial.print("global=");
  Serial.print(globalChannel);
  Serial.print(" board=");
  Serial.print(globalChannel < 12 ? "0x40" : "0x41");
  Serial.print(" channel=");
  Serial.print(localChannel(globalChannel));
  Serial.print(" cell=");
  Serial.print(cellForGlobalChannel(globalChannel));
  Serial.print(" dot=");
  Serial.print(dotForGlobalChannel(globalChannel));
}

void printStatus() {
  Serial.print("I2C 0x40 board1: ");
  Serial.println(i2cFound(BOARD1_ADDR) ? "FOUND" : "MISSING");
  Serial.print("I2C 0x41 board2: ");
  Serial.println(i2cFound(BOARD2_ADDR) ? "FOUND" : "MISSING");
  Serial.print("SELECTED ");
  printChannel(selectedChannel);
  Serial.print(" pulse=");
  Serial.println(selectedPulse);
}

void writePulseQuiet(byte globalChannel, int pulse) {
  if (globalChannel > 23) {
    return;
  }

  pulse = constrain(pulse, MIN_SAFE_PULSE, MAX_SAFE_PULSE);
  driverForGlobalChannel(globalChannel)->setPWM(localChannel(globalChannel), 0, pulse);
}

void setPulse(byte globalChannel, int pulse) {
  if (globalChannel > 23) {
    Serial.println("ERR CHANNEL 0-23");
    return;
  }

  pulse = constrain(pulse, MIN_SAFE_PULSE, MAX_SAFE_PULSE);
  writePulseQuiet(globalChannel, pulse);

  selectedChannel = globalChannel;
  selectedPulse = pulse;

  Serial.print("SET ");
  printChannel(globalChannel);
  Serial.print(" pulse=");
  Serial.println(pulse);
}

void selectChannel(byte globalChannel) {
  if (globalChannel > 23) {
    Serial.println("ERR CHANNEL 0-23");
    return;
  }

  selectedChannel = globalChannel;
  Serial.print("SELECTED ");
  printChannel(selectedChannel);
  Serial.print(" pulse=");
  Serial.println(selectedPulse);
}

void setCell1Offset(int offset) {
  for (byte dot = 0; dot < 6; dot++) {
    int pulse = CELL1_BASE_PULSE + CELL1_DIRECTION[dot] * offset;
    setPulse(dot, pulse);
    delay(CELL1_DEMO_DELAY_MS);
  }
}

void cell1OppositeDemo() {
  Serial.println("CELL1_OPPOSITE_DEMO_START");
  Serial.println("T0 decreases, T1/T2/T3/T4/T5 increase");

  for (int offset = CELL1_DEMO_STEP; offset <= CELL1_DEMO_AMOUNT; offset += CELL1_DEMO_STEP) {
    Serial.print("CELL1_OFFSET ");
    Serial.println(offset);
    setCell1Offset(offset);
    delay(CELL1_DEMO_DELAY_MS);
  }

  for (int offset = CELL1_DEMO_AMOUNT - CELL1_DEMO_STEP; offset >= 0; offset -= CELL1_DEMO_STEP) {
    Serial.print("CELL1_REVERSE_OFFSET ");
    Serial.println(offset);
    setCell1Offset(offset);
    delay(CELL1_DEMO_DELAY_MS);
  }

  Serial.println("CELL1_OPPOSITE_DEMO_DONE");
}

void rhythmResetT0ToT18() {
  for (byte channel = 0; channel < RHYTHM_CHANNEL_COUNT; channel++) {
    writePulseQuiet(channel, RHYTHM_BASE[channel]);
    delay(20);
  }
}

void rhythmRun(unsigned long runMs) {
  unsigned long startedAt = millis();
  byte channel = 0;

  while (millis() - startedAt < runMs) {
    int base = RHYTHM_BASE[channel];
    int lifted = base + RHYTHM_DIRECTION[channel] * RHYTHM_AMOUNT;

    writePulseQuiet(channel, lifted);
    delay(RHYTHM_ON_MS);
    writePulseQuiet(channel, base);
    delay(RHYTHM_OFF_MS);

    channel++;
    if (channel >= RHYTHM_CHANNEL_COUNT) {
      channel = 0;
    }
  }
}

void rhythmDemo() {
  Serial.println("RHYTHM_START T0_T23");
  Serial.println("RUN_1_3S");
  rhythmRun(RHYTHM_RUN_MS);
  rhythmResetT0ToT18();

  Serial.println("PAUSE_3S");
  delay(RHYTHM_LONG_PAUSE_MS);

  Serial.println("RUN_2_3S");
  rhythmRun(RHYTHM_RUN_MS);
  rhythmResetT0ToT18();

  Serial.println("PAUSE_1S");
  delay(RHYTHM_SHORT_PAUSE_MS);

  rhythmResetT0ToT18();
  Serial.println("RHYTHM_DONE");
}

void handleCommand(char *cmd) {
  if (strcmp(cmd, "S") == 0) {
    printStatus();
    return;
  }

  if (strcmp(cmd, "?") == 0) {
    Serial.print("SELECTED ");
    printChannel(selectedChannel);
    Serial.print(" pulse=");
    Serial.println(selectedPulse);
    return;
  }

  if (strcmp(cmd, "+") == 0) {
    setPulse(selectedChannel, selectedPulse + NUDGE);
    return;
  }

  if (strcmp(cmd, "-") == 0) {
    setPulse(selectedChannel, selectedPulse - NUDGE);
    return;
  }

  if (strcmp(cmd, "O1") == 0) {
    cell1OppositeDemo();
    return;
  }

  if (strcmp(cmd, "RHY") == 0) {
    rhythmDemo();
    return;
  }

  if (cmd[0] == 'C' && cmd[1] == ':') {
    selectChannel((byte)atoi(cmd + 2));
    return;
  }

  if (cmd[0] == 'P' && cmd[1] == ':') {
    setPulse(selectedChannel, atoi(cmd + 2));
    return;
  }

  if (cmd[0] == 'X' && cmd[1] == ':') {
    char *secondColon = strchr(cmd + 2, ':');
    if (secondColon == NULL) {
      Serial.println("ERR USE X:CHANNEL:PULSE");
      return;
    }

    *secondColon = '\0';
    byte globalChannel = (byte)atoi(cmd + 2);
    int pulse = atoi(secondColon + 1);
    setPulse(globalChannel, pulse);
    return;
  }

  Serial.println("ERR COMMANDS: S C:0 P:300 X:5:320 O1 RHY + - ?");
}

void readSerial() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c >= 'a' && c <= 'z') {
      c = c - 'a' + 'A';
    }

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

  Serial.println("FINGERTIP_CALIBRATE_READY_NO_AUTO_MOVE");
  printStatus();
  Serial.println("Commands: S C:0 P:300 X:5:320 O1 RHY + - ?");
}

void loop() {
  readSerial();
}

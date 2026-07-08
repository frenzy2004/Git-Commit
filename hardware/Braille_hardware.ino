#include <Arduino.h>

constexpr uint8_t MOTOR_COUNT = 4;
constexpr uint16_t STEPS_PER_TURN = 4096;
constexpr uint16_t HALF_TURN_STEPS = STEPS_PER_TURN / 2;
constexpr uint8_t STEP_DELAY_MS = 2;
constexpr unsigned long CHUNK_HOLD_MS = 5000;
constexpr float REST_ANGLE = 239.75;

const uint8_t MOTOR_PINS[MOTOR_COUNT][4] = {
  {13, 12, 14, 27},
  {26, 25, 33, 32},
  {23, 22, 21, 19},
  {18, 5, 4, 2}
};

const uint8_t HALF_STEP_SEQUENCE[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

struct CharacterAngle {
  char value;
  float angle;
};

const CharacterAngle CHARACTER_ANGLES[] = {
  {' ', REST_ANGLE},
  {'9', 338.75}, {'8', 333.25}, {'7', 327.75}, {'6', 322.25},
  {'5', 316.75}, {'4', 311.25}, {'3', 305.75}, {'2', 300.25},
  {'1', 294.75}, {'0', 289.25}, {'#', 283.75}, {'^', 278.25},
  {'?', 272.75}, {'.', 267.25}, {'-', 261.75}, {',', 256.25},
  {'\'', 250.75}, {'!', 245.25},
  {'z', 234.75}, {'y', 229.00}, {'x', 223.50}, {'w', 218.25},
  {'v', 212.50}, {'u', 207.50}, {'t', 201.63}, {'s', 195.88},
  {'r', 191.13}, {'q', 185.38}, {'p', 180.00}, {'o', 174.63},
  {'n', 168.88}, {'m', 164.13}, {'l', 158.38}, {'k', 152.50},
  {'j', 147.50}, {'i', 141.75}, {'h', 136.50}, {'g', 131.00},
  {'f', 125.25}, {'e', 120.25}, {'d', 114.25}, {'c', 108.50},
  {'b', 103.50}, {'a', 98.00}
};

long motorStepPosition[MOTOR_COUNT];
uint8_t motorPhase[MOTOR_COUNT] = {0, 0, 0, 0};

long stepsFromAngle(float angle) {
  return round((angle / 360.0) * STEPS_PER_TURN);
}

long wrapSteps(long steps) {
  steps %= STEPS_PER_TURN;
  return steps < 0 ? steps + STEPS_PER_TURN : steps;
}

long shortestDelta(long current, long target) {
  long delta = target - current;
  if (delta > HALF_TURN_STEPS) {
    delta -= STEPS_PER_TURN;
  } else if (delta < -HALF_TURN_STEPS) {
    delta += STEPS_PER_TURN;
  }
  return delta;
}

void driveCoils(uint8_t motor, uint8_t phase) {
  for (uint8_t pinIndex = 0; pinIndex < 4; pinIndex++) {
    digitalWrite(MOTOR_PINS[motor][pinIndex], HALF_STEP_SEQUENCE[phase][pinIndex]);
  }
}

void releaseMotor(uint8_t motor) {
  for (uint8_t pinIndex = 0; pinIndex < 4; pinIndex++) {
    digitalWrite(MOTOR_PINS[motor][pinIndex], LOW);
  }
}

void releaseAllMotors() {
  for (uint8_t motor = 0; motor < MOTOR_COUNT; motor++) {
    releaseMotor(motor);
  }
}

void stepMotor(uint8_t motor, int8_t direction) {
  motorPhase[motor] = (motorPhase[motor] + direction + 8) % 8;
  driveCoils(motor, motorPhase[motor]);
  motorStepPosition[motor] = wrapSteps(motorStepPosition[motor] + direction);
}

float angleForCharacter(char rawCharacter) {
  char value = tolower(rawCharacter);
  for (const CharacterAngle &entry : CHARACTER_ANGLES) {
    if (entry.value == value) {
      return entry.angle;
    }
  }
  return REST_ANGLE;
}

void moveMotorsTogether(const float targetAngles[MOTOR_COUNT]) {
  long remaining[MOTOR_COUNT];
  int8_t direction[MOTOR_COUNT];

  for (uint8_t motor = 0; motor < MOTOR_COUNT; motor++) {
    long targetSteps = wrapSteps(stepsFromAngle(targetAngles[motor]));
    motorStepPosition[motor] = wrapSteps(motorStepPosition[motor]);

    long delta = shortestDelta(motorStepPosition[motor], targetSteps);
    direction[motor] = delta >= 0 ? 1 : -1;
    remaining[motor] = abs(delta);
  }

  bool anyMoving = true;
  while (anyMoving) {
    anyMoving = false;

    for (uint8_t motor = 0; motor < MOTOR_COUNT; motor++) {
      if (remaining[motor] > 0) {
        stepMotor(motor, direction[motor]);
        remaining[motor]--;
        anyMoving = true;
      }
    }

    if (anyMoving) {
      delay(STEP_DELAY_MS);
    }
  }

  for (uint8_t motor = 0; motor < MOTOR_COUNT; motor++) {
    motorStepPosition[motor] = wrapSteps(stepsFromAngle(targetAngles[motor]));
  }
}

void showFourCharacters(const String &chunk) {
  float targets[MOTOR_COUNT] = {
    angleForCharacter(chunk[0]),
    angleForCharacter(chunk[1]),
    angleForCharacter(chunk[2]),
    angleForCharacter(chunk[3])
  };

  moveMotorsTogether(targets);
}

String normalizeChunk(String chunk) {
  chunk.toLowerCase();
  chunk.replace("\r", "");

  while (chunk.length() < MOTOR_COUNT) {
    chunk += ' ';
  }

  if (chunk.length() > MOTOR_COUNT) {
    chunk = chunk.substring(0, MOTOR_COUNT);
  }

  return chunk;
}

void setup() {
  for (uint8_t motor = 0; motor < MOTOR_COUNT; motor++) {
    for (uint8_t pinIndex = 0; pinIndex < 4; pinIndex++) {
      pinMode(MOTOR_PINS[motor][pinIndex], OUTPUT);
    }
    motorStepPosition[motor] = stepsFromAngle(REST_ANGLE);
    motorPhase[motor] = 0;
    releaseMotor(motor);
  }

  Serial.begin(115200);
  Serial.setTimeout(50);
}

void loop() {
  if (Serial.available() <= 0) {
    return;
  }

  String chunk = normalizeChunk(Serial.readStringUntil('\n'));
  showFourCharacters(chunk);
  delay(CHUNK_HOLD_MS);
  releaseAllMotors();
}

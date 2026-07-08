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

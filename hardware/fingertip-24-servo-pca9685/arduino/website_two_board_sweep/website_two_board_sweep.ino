#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x41);

int servoMin = 150;
int servoMax = 600;
int servoFrequency = 50;

void setup()
{
  pwm1.begin();
  pwm2.begin();
  pwm1.setOscillatorFrequency(27000000);
  pwm2.setOscillatorFrequency(27000000);
  pwm1.setPWMFreq(servoFrequency);
  pwm2.setPWMFreq(servoFrequency);
}

void loop()
{
  for (int i = 0; i <= 5; i++)
  {
    for (int pulseLength = servoMin; pulseLength <= servoMax; pulseLength++)
    {
      pwm1.setPWM(i, 0, pulseLength);
      pwm2.setPWM(i, 0, pulseLength);
      delay(1);
    }

    delay(100);

    for (int pulseLength = servoMax; pulseLength >= servoMin; pulseLength--)
    {
      pwm1.setPWM(i, 0, pulseLength);
      pwm2.setPWM(i, 0, pulseLength);
      delay(1);
    }

    delay(100);
  }

  delay(500);
}

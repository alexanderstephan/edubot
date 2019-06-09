#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class WRSK_UltrasonicSensor {
  public:
    WRSK_UltrasonicSensor(int _echoPin, int _triggerPin);
    WRSK_UltrasonicSensor(int _echoPin, int _triggerPin, int dbgLevel);
    float read(void);
    float readcm(void);
    float readinch(void);

  private:
    int echoPin;
    int triggerPin;
    int dbgLevel;
    int readRAW(void);
    void initUltrasonicSensor(void);
    float microsecondsToInches(int microsecond);
    float microsecondsToCentimeters(int microsecond);
};

#endif /* ULTRASONIC_H */
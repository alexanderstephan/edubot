#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


#include "SR04.h" 

WRSK_UltrasonicSensor::WRSK_UltrasonicSensor(int _echoPin, int _triggerPin)
{
  this->echoPin = _echoPin;
  this->triggerPin = _triggerPin;
  this->dbgLevel = 0;
  initUltrasonicSensor();
}

WRSK_UltrasonicSensor::WRSK_UltrasonicSensor(int _echoPin, int _triggerPin, int _dbgLevel)
{
  this->echoPin = _echoPin;
  this->triggerPin = _triggerPin;
  this->dbgLevel = _dbgLevel;
  initUltrasonicSensor();
}

float WRSK_UltrasonicSensor::read(void)
{
  return readcm();
}

float WRSK_UltrasonicSensor::readcm(void)
{
  int pulsein;
  float distance;

  pulsein = readRAW();
  distance = microsecondsToCentimeters(pulsein);
  if (distance < 4 && distance > 60)
  {
    distance = -1;
  }/*
  if (this->dbgLevel > 1) 
  {
    Serial.print("Distance: ");
    Serial.print(distance, DEC);
    Serial.println(" cm");
  }*/
  return distance;
} 

float WRSK_UltrasonicSensor::readinch(void)
{
  int pulsein;
  float distance;

  pulsein = readRAW();
  distance = microsecondsToInches(pulsein);
  if (distance < 2 && distance > 24)
  {
    distance = -1;
  }
  if (this->dbgLevel > 1) 
  {
    Serial.print("Distance: ");
    Serial.print(distance, DEC);
    Serial.println(" in");
  }
  return distance;
} 


int WRSK_UltrasonicSensor::readRAW(void)
{
  int pulsein;
  // The HC-SR04 is triggered by a HIGH pulse of 10 or more microseconds.
  digitalWrite(this->triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(this->triggerPin, LOW);
 
  // The same pin is used to read the signal from the HC-SR04 a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pulsein = pulseIn(this->echoPin, HIGH);
  return pulsein;
} 

void WRSK_UltrasonicSensor::initUltrasonicSensor(void)
{
  pinMode(this->triggerPin, OUTPUT);
  pinMode(this->echoPin, INPUT);
  digitalWrite(this->triggerPin, LOW);
}

float WRSK_UltrasonicSensor::microsecondsToInches(int microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  return microseconds / 74.0 / 2.0;
}
 
float WRSK_UltrasonicSensor::microsecondsToCentimeters(int microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29.0 / 2.0;
}

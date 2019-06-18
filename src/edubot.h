#ifndef EDUBOT_EDUBOT_H
#define EDUBOT_EDUBOT_H

/* Servo pin */
#define SERVO_PWM 15

/* Ultrasonic sensor pins */
#define ECHO_PIN 12
#define TRIG_PIN 13

/* Ultrasonic sensor */
void getDistance();

/* Robot movement */
void initServo();
void turnServo();
void collisionHandling();
void driveSpiral();
void setMode();

/* Server */
String prepareHtmlPage();
void handleGet();
void handleNotFound();

#endif //EDUBOT_EDUBOT_H

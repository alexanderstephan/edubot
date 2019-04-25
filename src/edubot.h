#ifndef EDUBOT_EDUBOT_H
#define EDUBOT_EDUBOT_H

/* Servo pin */
#define SERVO_PWM 12

/* Ultrasonic sensor pins */
#define ECHO_PIN 14
#define TRIG_PIN 16

/* Ultrasonic sensor */
void getDistance();

/* Robot movement */

void initServo();
void turnServo();
void collisionHandling();
void driveSpiral();

/* Server */
//String prepareHtmlPage();
void handleGet();
void handleNotFound();

#endif //EDUBOT_EDUBOT_H


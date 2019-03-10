#ifndef EDUBOT_EDUBOT_H
#define EDUBOT_EDUBOT_H

typedef enum {LEFT, RIGHT} direction_t;

/****utrasonic sensor****/
void getDistance();

/****robot movement****/
void driveForward();
void changeDirA();
void changeDirB();
void driveBackward();
void stopWheel(bool left);
void stopAll();
void turnDegree();
void turnTime();
void turnLeft();
void turnRight();
void turnServo();
void collisionHandling();
void driveSpiral();

/****server****/
String prepareHtmlPage();
void handleGet();
void handleNotFound();


#endif //EDUBOT_EDUBOT_H

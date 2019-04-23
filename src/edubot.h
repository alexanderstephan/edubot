#ifndef EDUBOT_EDUBOT_H
#define EDUBOT_EDUBOT_H

/* Motor pins */
#define MOTOR_A_SPEED 13  // enA
#define MOTOR_A_ENABLE1 2 // in1
#define MOTOR_A_ENABLE2 0  // in2

#define MOTOR_B_SPEED 15  // enB
#define MOTOR_B_ENABLE1 5  // in3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
#define MOTOR_B_ENABLE2 4  // in42
 
/* Servo pin */
#define SERVO_PWM 12

/* Ultrasonic sensor pins */
#define ECHO_PIN 14
#define TRIG_PIN 16

typedef enum {LEFT, RIGHT, FORWARD, BACKWARD, NONE} direction_t;
typedef enum {AUTO, SPIRAL, IDLE} drivingMode_t;

/* Ultrasonic sensor */
void getDistance();

/* Robot movement */
void getDirection(direction_t);
void setMode(drivingMode_t alteredMode);
void setDefaultSpeed();
void initServo();
void driveForward();
void driveBackward();
void changeDirA();
void changeDirB();
void stopWheel(bool left);
void stopAll();
void turnDir(direction_t dir, int time);
void turnLeft();
void turnRight();
void turnServo();
void collisionHandling();
void driveSpiral();

/* Server */
String prepareHtmlPage();
void handleGet();
void handleNotFound();

typedef struct drivingState {
    direction_t dir;
    int speedA;
    int speedB;
    drivingMode_t mode;
} drivingState_t;

#endif //EDUBOT_EDUBOT_H


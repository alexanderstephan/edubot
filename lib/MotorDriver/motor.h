#ifndef EDUBOT_MOTOR_H
#define EDUBOT_MOTOR_H

/* Motor pins */
#define MOTOR_A_SPEED 13  // enA
#define MOTOR_A_ENABLE1 2 // in1
#define MOTOR_A_ENABLE2 0  // in2

#define MOTOR_B_SPEED 15  // enB
#define MOTOR_B_ENABLE1 5  // in3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
#define MOTOR_B_ENABLE2 4  // in42
 
typedef enum {LEFT, RIGHT, FORWARD, BACKWARD, NONE} direction_t;
typedef enum {AUTO, FOLLOW, IDLE} drivingMode_t;

typedef struct drivingState {
    direction_t dir;
    int speedA;
    int speedB;
    drivingMode_t mode;
} drivingState_t;

void getDirection(direction_t);
void setMode(drivingMode_t alteredMode);
void driveForward();
void driveBackward();
void changeDirA();
void changeDirB();
void handBrake();
void turnDir(direction_t dir, int time);
void turnLeft();
void turnRight();
void readDirection();
void init(drivingState_t *state);

#endif
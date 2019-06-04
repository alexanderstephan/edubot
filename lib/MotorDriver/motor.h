#ifndef EDUBOT_MOTOR_H
#define EDUBOT_MOTOR_H

/* Motor pins */
#define MOTOR_A_SPEED 13  // enA
#define MOTOR_A_ENABLE1 2 // in1
#define MOTOR_A_ENABLE2 0  // in2

#define MOTOR_B_SPEED 15  // enB
#define MOTOR_B_ENABLE1 5  // in3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
#define MOTOR_B_ENABLE2 4  // in42
 
typedef enum {LEFT, RIGHT, FORWARD, BACKWARD, NONE} direction_t;    // To keep track of current direction
typedef enum {AUTO, FOLLOW, IDLE} drivingMode_t;    // To keep track of the three driving modes

typedef struct drivingState {
    direction_t dir; // Check out direction
    int speedA; // Monitor current speed
    int speedB;
    int prevA;  // Save speed in these variables to restore previous speeds
    int prevB;
    drivingMode_t mode; /// Check out driving mode
} drivingState_t;

void driveWheels(int valLeft, int valRight);
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
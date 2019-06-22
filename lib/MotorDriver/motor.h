#ifndef EDUBOT_MOTOR_H
#define EDUBOT_MOTOR_H

/* Motor pins */
#define MOTOR_A_SPEED       5
#define MOTOR_A_DIR         0

#define MOTOR_B_SPEED       4
#define MOTOR_B_DIR         2

/* Driving states */
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

/* Function declarations */
void driveWheels(int valLeft, int valRight);
void getDirection(direction_t);
void driveForward();
void driveBackward();
void changeDirA();
void changeDirB();
void handBrake();
void turnDir(direction_t steerDir, int time);
void readDirection();
void init(drivingState_t *state);
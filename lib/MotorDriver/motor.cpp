#include <Arduino.h>
#include "motor.h"
#include <stdio.h>

#define DEBUG_LEVEL 2   // Global debug variable that determines the amount of logging data

int debugLevel = DEBUG_LEVEL; 

drivingState_t *dState = NULL;  // We need to work the the pointer of the struct

void init(drivingState_t *state){
    dState=state;
}

/* ------------------------------------------------------
            Core feature: General motor logic
-------------------------------------------------------*/

// This function reads to sped values and sets the driving direction according to the algebraic sign
// Maybe this needs refactoring
void driveWheels(int valLeft, int valRight) {
    if (valLeft < 0) {
        digitalWrite(MOTOR_A_ENABLE1, HIGH);
        digitalWrite(MOTOR_A_ENABLE2, LOW);
        dState->speedA = valLeft;
    } else {
        digitalWrite(MOTOR_A_ENABLE1, LOW);
        digitalWrite(MOTOR_A_ENABLE2, HIGH);
        dState->speedA = valLeft;
    }
    if (valRight < 0) {
        digitalWrite(MOTOR_B_ENABLE1, HIGH);
        digitalWrite(MOTOR_B_ENABLE2, LOW);
        dState->speedB = valRight;
    } else {
        digitalWrite(MOTOR_B_ENABLE1, LOW);
        digitalWrite(MOTOR_B_ENABLE2, HIGH);
        dState->speedB = valRight;
    }
    if (valLeft == 0) {
        digitalWrite(MOTOR_A_ENABLE1, HIGH);
        digitalWrite(MOTOR_A_ENABLE2, HIGH);
        dState->speedB  = 0;

    }
    if (valRight == 0) {
        digitalWrite(MOTOR_B_ENABLE1, HIGH);
        digitalWrite(MOTOR_B_ENABLE2, HIGH);
        dState->speedB = 0;
    }
    analogWrite(MOTOR_A_SPEED, abs(dState->speedA));
    analogWrite(MOTOR_B_SPEED, abs(dState->speedB));
}

// Read current speeds and check wether it's negative or positive. Set driving state accordingly
void readDirection() {
    if (dState->speedA > 0 && dState->speedB > 0) { // If both are positive the robot is driving forward
        dState->dir = FORWARD;
         if (debugLevel > 1) {
            Serial.println("Forward");
        }
    }
    else if (dState->speedA < 0  && dState->speedB < 0) { // If both are negative the robot is driving backward
        dState->dir = BACKWARD;
        if (debugLevel > 1) {
            Serial.println("Backward");
        }
    }
    else if (dState->speedA > 0 && dState->speedB < 0) { // If the left wheel is turning forward and right is moving backward, robot does a right turn
        dState->dir = RIGHT;
        if (debugLevel > 1) {
            Serial.println("Right");
        }
    }
    else if (dState->speedA < 0 && dState->speedB > 0) { // If the left wheel is turning backward and right is moving forward, robot does a right turn
        dState->dir = LEFT;
        if (debugLevel > 1) {
            Serial.println("Left  ");
        }
    }
    else if (dState->speedA == 0 && dState->speedB == 0) { // If both speeds are zero, there is none
        dState->dir = NONE;
    }
}

/* ------------------------------------------------------
        Basic robot controls via web interface
-------------------------------------------------------*/

void driveForward() {
    if(dState->speedA == 0 && dState->speedB == 0) {    // If the speed is zero, restore the previous speed
        driveWheels(abs(dState->prevA), abs(dState->prevB));
    }
    else  {
        driveWheels(abs(dState->speedA),abs(dState->speedB));   // Otherwise set the absolute speed values
    }
    if(debugLevel > 1) {
        Serial.println("Driving forward");
    }
}

void driveBackward() {
    if(dState->speedA == 0 && dState->speedB == 0) {
        driveWheels(-abs(dState->prevA), -abs(dState->prevB));  // If the speed is zero, restore the previous speed
    }
    else {
        driveWheels(-abs(dState->speedA),-abs(dState->speedB)); // Otherwise set the negative absolute speed values
    }
    if(debugLevel > 1) {
        Serial.println("Driving backwards");
    }
}

void handBrake() {
    // Set all motor pins on HIGH
    dState->prevA = dState->speedA;
    dState->prevB = dState->speedB;
    driveWheels(0,0);

    if( debugLevel > 1) {
        Serial.println("Robot is stopping...");
    }
}

// In order to turn the robot we need a function that changes the wheel direction
void changeDirA() {
    driveWheels(-abs(dState->speedA),abs(dState->speedB)); // We need to take the absolute value to make sure we have the same reference
}

void changeDirB() {
     driveWheels(abs(dState->speedA),-abs(dState->speedB)); // See above
}


// First of all declare the general function for turning movements. 
// The turn depends on the parameters direction and turning time
void turnDir(direction_t dir, int time) {
    if(dState->dir == FORWARD) {
        if(dir == LEFT) {
            if( debugLevel > 1) {
                Serial.println("Turning left");
            }
            driveWheels((0.5*(dState->speedA)), dState->speedB);   // Change orientation of the left wheel
            delay(time);    // Wait for the turn
            driveWheels(dState->prevA, dState->speedB);
        } else if(dir == RIGHT) {
            if( debugLevel > 1) {
                Serial.println("Turning right");
            }
            driveWheels(dState->speedA, 0.5*(dState->speedB));   // Change orientation of the left wheel
            delay(time);    // Wait for the turn
            driveWheels(dState->speedA, dState->prevA);          // Restore speeds
        }        
    } else if(dState->dir == BACKWARD) {
        if(dir == LEFT) {
            if( debugLevel > 1) {
                Serial.println("Turning left");
            }
            driveWheels((0.5*(dState->speedA)), dState->speedB);   // Change orientation of the left wheel
            delay(time);    // Wait for the turn
            driveWheels(dState->prevA, dState->speedB);
        } else if(dir == RIGHT) {
            if( debugLevel > 1) {
                Serial.println("Turning right");
            }
            driveWheels(dState->speedA, 0.5*(dState->speedB));   // Change orientation of the left wheel
            delay(time);    // Wait for the turn
            driveWheels(dState->speedA, dState->prevB);         // Restore speeds  
        }
    } else if (dState->dir == NONE) {
        if (dir == LEFT) {
            driveWheels(700, 700);
            changeDirA();
            delay(time);
        } else if (dir == LEFT){
            driveWheels(700, 700);
            changeDirB();
            delay(time);
        }       
    } else {
        Serial.println("Error reading direction!");
    }
}

// These two function make the robot turn in a fixed direction and degree
void turnRight() {
    // Since there is no speed set already, we need to set it manually
    if(dState->dir == FORWARD) {
        turnDir(RIGHT,300);
        handBrake();
        delay(200);
        driveForward();
    }
    else if(dState->dir == BACKWARD) {
        turnDir(RIGHT,300);
        handBrake();
        delay(200);
        driveBackward();
    }
    else {
        driveWheels(800, 800);
        turnDir(RIGHT,200);
        handBrake();
    }
}

// Same principle as above
void turnLeft() {
    if(dState->dir == FORWARD) {
        turnDir(LEFT,300);
        handBrake();
        delay(200);
        driveForward();
    }
    else if(dState->dir == BACKWARD) {
        turnDir(LEFT,300);
        handBrake();
        delay(200);
        driveBackward();
    }
    else {
        driveWheels(800, 800);
        turnDir(LEFT,200);
        handBrake();
    }
}

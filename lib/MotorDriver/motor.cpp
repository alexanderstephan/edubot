#include <Arduino.h>
#include "motor.h"
#include <stdio.h>

#define DEBUG_LEVEL 2

int debugLevel = DEBUG_LEVEL;
drivingState_t *dState = NULL; 

void driveWheels(int valRight, int valLeft) {
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

void readDirection() {
    if (dState->speedA > 0 && dState->speedB > 0) {
        dState->dir = FORWARD;
         if (debugLevel > 1) {
            Serial.println("FORWARD");
        } 
    }
    else if (dState->speedA < 0  && dState->speedB < 0) {
        dState->dir = BACKWARD;
        if (debugLevel > 1) {
            Serial.println("BACKWARD");
        }
    }
    else if (dState->speedA > 0 && dState->speedB < 0) {
        dState->dir = RIGHT;
        if (debugLevel > 1) {
            Serial.println("RIGHT");
        }   
    }
    else if (dState->speedA < 0 && dState->speedB > 0) {
        dState->dir = LEFT;
        if (debugLevel > 1) {
            Serial.println("LEFT");
        }
    }
    else {
        dState->dir = NONE;
    }
}

void driveForward() {
    driveWheels(abs(dState->speedA),abs(dState->speedB));

    if(debugLevel > 1) {
        Serial.println("Driving forward");
    }
}

void driveBackward() {
        driveWheels(-abs(dState->speedA),-abs(dState->speedB));
    if(debugLevel > 1) {
        Serial.println("Driving backwards");
    }
}

// Change direction, default is wheel driving forward
void changeDirA() {
    driveWheels(dState->speedA,-dState->speedB);
}

void changeDirB() {
     driveWheels(-dState->speedA,dState->speedB);
}

void stopWheel(bool left) {
    if(left) {
        digitalWrite(MOTOR_A_ENABLE1, HIGH);
        digitalWrite(MOTOR_A_ENABLE2, HIGH);
    }
    else {
        digitalWrite(MOTOR_B_ENABLE1, HIGH);
        digitalWrite(MOTOR_B_ENABLE2, HIGH);
    }
}

void handBrake() {
    // Set both motor pins on HIGH
    stopWheel(true);
    stopWheel(false);
    if( debugLevel > 1) {
    Serial.println("Robot is stopping...");
    }
}

void turnDir(direction_t dir, int time) {
    // If function argument equals LEFT, perform a right turn
    if(dir==LEFT) {
        if( debugLevel > 1) {
        Serial.println("Turning left");
        }
        changeDirA();
        delay(time);
    }
    // If function argument is RIGHT, perform a right turn
    else if(dir==RIGHT) {
        if( debugLevel > 1) {
        Serial.println("Turning right");
        }
        changeDirB();
        delay(time);
    }
    else {
        if( debugLevel > 1) {
        Serial.println("Error reading direction");
        }
    }
}

void turnRight() {
    if(dState->dir == FORWARD) {
        turnDir(RIGHT,250);
        handBrake();
        delay(200);
        driveForward();
    }
    else if(dState->dir == BACKWARD) {
        turnDir(RIGHT,250);
        handBrake();
        delay(200);
        driveBackward();
    }
    else {
        turnDir(RIGHT,250);
        handBrake();
    }
}

void turnLeft() {
    if(dState->dir == FORWARD) {
        turnDir(LEFT,250);
        handBrake();
        delay(200);
        driveForward();
    } 
    else if(dState->dir == BACKWARD) {
        turnDir(LEFT,250);
        handBrake();
        delay(200);
        driveBackward();
    } 
    else {
        turnDir(RIGHT,250);
        handBrake();
    }
}

void init(drivingState_t *state){
    dState=state;
}
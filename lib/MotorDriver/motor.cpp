#include <Arduino.h>
#include "motor.h"
#include <stdio.h>

#define DEBUG_LEVEL 2
int debugLevel = DEBUG_LEVEL;

drivingState_t *dState = NULL;

void init(drivingState_t *state){
    dState=state;
}

/*  Motor logic   */

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

void readDirection() {
    if (dState->speedA > 0 && dState->speedB > 0) {
        dState->dir = FORWARD;
         if (debugLevel > 1) {
            Serial.println("Forward");
        }
    }
    else if (dState->speedA < 0  && dState->speedB < 0) {
        dState->dir = BACKWARD;
        if (debugLevel > 1) {
            Serial.println("Backward");
        }
    }
    else if (dState->speedA > 0 && dState->speedB < 0) {
        dState->dir = RIGHT;
        if (debugLevel > 1) {
            Serial.println("Right");
        }
    }
    else if (dState->speedA < 0 && dState->speedB > 0) {
        dState->dir = LEFT;
        if (debugLevel > 1) {
            Serial.println("Left  ");
        }
    }
    else if (dState->speedA == 0 && dState->speedB == 0) {
        dState->dir = NONE;
    }

}

/*  Basic movements  */

void driveForward() {
    if(dState->speedA == 0 && dState->speedB == 0) {
        driveWheels(abs(dState->prevA), abs(dState->prevB));
    }
    else  {
        driveWheels(abs(dState->speedA),abs(dState->speedB));
    }
    if(debugLevel > 1) {
        Serial.println("Driving forward");
    }
}

void driveBackward() {
    if(dState->speedA == 0 && dState->speedB == 0) {
        driveWheels(-abs(dState->prevA), -abs(dState->prevB));
    }
    else  {
        driveWheels(-abs(dState->speedA),-abs(dState->speedB));
    }
    if(debugLevel > 1) {
        Serial.println("Driving backwards");
    }
}

// Change direction, default is wheel driving forward
void changeDirA() {
    driveWheels(-abs(dState->speedA),abs(dState->speedB));
}

void changeDirB() {
     driveWheels(abs(dState->speedA),-abs(dState->speedB));
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
        driveWheels(800, 800);
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
        driveWheels(800, 800);
        turnDir(LEFT,250);
        handBrake();
    }
}

/*----------------------------------------------------------------------------------
* NAME: Edubot
* AUTHOR: Alexander Stephan
* DESCRIPTION: Wirelessly controlled robot with the ability to drive autonomously
* SPECIAL THANKS: Max W., Markus Knapp and Robotfreak
----------------------------------------------------------------------------------*/

#include <Arduino.h>
#include <Servo.h>  // Servo library
#include <SR04.h>   // Ultra sonic library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>     // File system
#include "edubot.h" // Pins are defined here
#include <motor.h>  // Small library for handling the H-Bridge

// Credentials of the access point
#define HOST "edubot"
#define PASSWORD "12345678"

// Define speed values
#define MINIMUM_SPEED 300
#define DEFAULT_SPEED 512
#define MAX_SPEED 1023
#define TURN_SPEED 600

// Define servo values
#define SERVO_DEFAULT 90
#define SERVO_RIGHT 120
#define SERVO_LEFT 60

// Define tresholds
#define HAND_DISTANCE 15
#define TOO_CLOSE 4
#define MIN_DISTANCE 10

// Define debug level for enabling serial output
#define DEBUG_LEVEL 2

int debug_Level = DEBUG_LEVEL;
int currentSpeed = 0; // Make a variable to store the current speed, speed at the start should be zero
float distance;       // Global variable that keeps track of the current speed

// Initialize objects
Servo servo1;
WRSK_UltrasonicSensor us(ECHO_PIN, TRIG_PIN, DEBUG_LEVEL);

// Start HTTP server at port 80
// Adress is probably 192.168.178.4
ESP8266WebServer server(80); 

// Initialize driving states
drivingState_t d_State = {
    NONE,       // Direction
    0,          // Speed A
    0,          // Speed B
    0,          // Previous speed A
    0,          // Previous speed B
    IDLE        // Mode
};

// Force driving modes
void setMode(drivingMode_t alteredMode) {
    if(alteredMode == d_State.mode) {   // On: AUTO == AUTO
        handBrake();
        servo1.write(SERVO_DEFAULT);
        d_State.mode = IDLE;
    }
    else {
        d_State.mode = alteredMode;     // Off: alteredMode = IDLE
                                        // d.State.mode = AUTO;
    }
}

// Useful debug function to read the current the distance
void getDistance() {
    distance = us.read();
    Serial.print(distance);
    Serial.println("cm");
    delay(50);
}

// Bring servo back in a default middle position
void initServo(int servoPos) {
    servo1.write(servoPos);
}

// Just a function to continously rotate the servo within a certain degree
void turnServo(int degree) {
    int pos = SERVO_DEFAULT;        // Start rotation from the middle
    int turnDegree = degree;
    do {
            pos++;
            servo1.write(pos);
            distance = us.read();
            delay(25);
        } while(pos <= (SERVO_DEFAULT + turnDegree) && distance > 10.0);

     do {
            pos--;
            servo1.write(pos);
            distance = us.read();
            delay(25);
        } while(pos >= (SERVO_DEFAULT - turnDegree) && distance > 10.0);
}                                                                                                                   

/* ------------------------------------------------------
            Core feature: Obstacle avoidance
-------------------------------------------------------*/

int seekingPositionWithClosestDanger() {
    // Stop the robot
    handBrake();

    int servoPos;
    int minDistance = 100;
    int minServoPos = 0;

    Serial.println("Moving servo from left to right!");

    for(servoPos = SERVO_RIGHT; servoPos >= SERVO_LEFT; servoPos--) {
        servo1.write(servoPos);
        delay(10);  // Wait until servo position is reached
        if (distance < minDistance) {
            if (distance > 0.1) {       // Avoid invalid readings
                minDistance = distance; // Current value is new minimum
            }
            minServoPos = servoPos;     // Save servo position
        }
    }
    servo1.write(minServoPos);
    Serial.print("Minimale Distanz:");
    Serial.println(minDistance);
    Serial.print("Servo Position:");
    Serial.println(minServoPos);
    return minServoPos;
}

void avoidObstacle(boolean Direction) {
    handBrake();
    delay(500);
    if (Direction == LEFT) {
        driveWheels(TURN_SPEED,-TURN_SPEED); // See above
    }
    else if (Direction == RIGHT) {
        driveWheels(-TURN_SPEED,TURN_SPEED); // See above
    }
    else {
        Serial.println("Error reading direction!");
    }
}

void collisionHandling() {
    int dangerPos;
    distance = us.read();

    // If ultrasonic distance is less than 10 perform a obstacle avoidance routine, else proceed driving
    if (distance > 0.1) {   // Avoid invalid readings
        if (distance < MIN_DISTANCE) {
            // Wait if the sensor value stabilizes
            Serial.println("Recognized potential danger!");
            Serial.println("Seeking danger position!");
            dangerPos = seekingPositionWithClosestDanger();
            Serial.print("Obstacle at position ");
            Serial.println(dangerPos);
            Serial.println("Stopped robot!");

            if(dangerPos <= SERVO_DEFAULT) {
                Serial.println("Obstacle on the left1");
                avoidObstacle(LEFT);
                do {
                    delay(50);
                } while (us.read() < (MIN_DISTANCE + 3));
            }

            if(dangerPos > SERVO_DEFAULT) {
                Serial.println("Obstacle on the left1");
                avoidObstacle(RIGHT);
                do {
                   delay(50);
                } while (us.read() < (MIN_DISTANCE + 3));
            }

            delay(200);

            Serial.println("Avoided obstacle!");
        // Continue driving while searching for obstacles
        } else if (distance > 10.0) {
            Serial.println("More or less out of sight!");
            driveWheels(DEFAULT_SPEED, DEFAULT_SPEED);
            turnServo(40);
        } else if (distance > 20.0) {
            Serial.println("Obstacle out of sight!");
            driveWheels(MAX_SPEED, MAX_SPEED);
            turnServo(30);
        }
        delay(10);
    }
}


/* ---------------------------------------------------------
    Pet mode inspired by Markus Knapp (Work in progress)
----------------------------------------------------------*/

int searchHand() {
    distance = us.read();
    int handPosition =  0;
    handBrake();
    delay(500);
    initServo(SERVO_RIGHT);

    do {
        for (int servoPosition = SERVO_RIGHT; servoPosition >= SERVO_LEFT; servoPosition--) {
            // Set servo position
            servo1.write(servoPosition);

            // Make sure servo isn't turning too fast
            delay(10);

            // Update sensor value
            distance = us.read();

            // If hand position detected return it as an integer
            if (distance <= HAND_DISTANCE) {
                handPosition = servoPosition;
                return handPosition;
            }
        }
        // Same thing but in the other direction
        for (int servoPosition = SERVO_LEFT; servoPosition <= SERVO_RIGHT; servoPosition++) {
            servo1.write(servoPosition);
            delay(10);
            distance = us.read();

            if (distance <= HAND_DISTANCE) {
                handPosition = servoPosition;
                return handPosition;
            }
        }
    } while (distance > HAND_DISTANCE); // Search while hand is not within reach
    return 0;
}

void followHand() {
    initServo(SERVO_DEFAULT);
    analogWrite(MOTOR_A_SPEED, MAX_SPEED);
    analogWrite(MOTOR_B_SPEED, MAX_SPEED);
    driveForward();

    do {
        distance = us.read();
        delay(10);
    } while ((distance <= HAND_DISTANCE) && (distance <= TOO_CLOSE));
    
    handBrake();
    
    delay(1000);
}

void turnTowardsHand() {

}

/* ------------------------------------------------------
 Everything related to setting up the server and website
-------------------------------------------------------*/

// This function just reads the web files from the data folder and returns them as a string
String prepareHtmlPage() {
        String htmlPage;                                // Init string
        File f = SPIFFS.open("/edubot.html","r");       // Open file
        // Check for an error
        if (!f) {
            if(debug_Level > 1) {
            Serial.println("Error reading .html file!");
            }
        }

        // Read file into string
        else {
            htmlPage = f.readString();
            if( debug_Level > 1) {
            Serial.println("Reading files succesfully!");
            }
        }
        f.close();  // Close file
        return htmlPage;
}

// This one just listens to any possible changed server arguments
// Also sends the above generated html string to the server
// CSS and Java script are provided by serveStatic() in setup (Consistency?)
// Primarely used for the velocity slider for now
void handleGet(){
    if(server.args()>0) { // If there is an valid argument
        if(server.hasArg("speed")) {
            if (debug_Level > 2){
                Serial.println(server.arg("speed").toInt());      // Log current speed
            }
            // Convert string to integer and set motor speed accordingly
            d_State.speedA = server.arg("speed").toInt();
            d_State.speedB = server.arg("speed").toInt();
            // Set current speed values
            analogWrite(MOTOR_A_SPEED, abs(d_State.speedA));
            analogWrite(MOTOR_B_SPEED, abs(d_State.speedB));
        }
    } else {
        server.send(200, "text/html", prepareHtmlPage());
    }   
}

void handleAuto() {
    if(server.args()>0) {
        if(server.hasArg("auto")) {
            String currentStatus;
            String buttonState;
            Serial.println("Button has been pressed!");
            buttonState = server.arg("auto");

            if(buttonState == "0") {
                d_State.mode = IDLE;
                handBrake();
                Serial.println("Auto turned off!");
            }
            else {
                d_State.mode = AUTO;
                Serial.println("Auto turned on!");
            }

            currentStatus = d_State.mode;
            server.send(200, "text/html", currentStatus);
        }
    }
}

void handleFollow() {
    if(server.args()>0) {
        if(server.hasArg("follow")) {
            String currentStatus;
            String buttonState;
            Serial.println("Button has been pressed!");
            buttonState = server.arg("follow");

            if(buttonState == "0") {
                d_State.mode = IDLE;
                handBrake();
                Serial.println("Auto turned off!");
            }
            else {
                d_State.mode = FOLLOW;
                Serial.println("Auto turned on!");
            }

            currentStatus = d_State.mode;
            server.send(200, "text/html", currentStatus);
        }
    }
}

void updateMode() {
    String currentStatus;
    if(d_State.mode == IDLE) {
        currentStatus = "OFF";
    }
    else if(d_State.mode == AUTO) {
        currentStatus = "ON";
    }
    else {
        Serial.println("Error!");
    }
    server.send(200, "text/plane", currentStatus);
}

// In case an error happens,send some whoopsie message to the server
void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for (uint8_t i=0; i<server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }

    server.send(404, "text/plain", message);
}

/* ------------------------------------------------------
                    Basic setup
-------------------------------------------------------*/

void setup() {
    // Initialize serial port
    Serial.begin(115200);

    // Start file system
    SPIFFS.begin();

    // Initialize servo pin
    servo1.attach(15);

    // Initialize struct with adress
    init(&d_State);

    // Set all motor pins as outout
    pinMode(MOTOR_A_SPEED, OUTPUT);
    pinMode(MOTOR_A_DIR, OUTPUT);

    pinMode(MOTOR_B_SPEED, OUTPUT);
    pinMode(MOTOR_B_DIR, OUTPUT);

    // Use access point mode
    WiFi.mode(WIFI_AP);

    // Define its name and password
    WiFi.softAP(HOST, PASSWORD);
    if (debug_Level > 1) {
        Serial.println("");

        // Print IP adress
        Serial.println("");
        Serial.print("Name of Access Point is:");
        Serial.println("edubot");
        Serial.print("IP address: ");
        Serial.println(WiFi.softAPIP());
    }

    // Handle server requests
    server.on("/",HTTP_GET,handleGet);
    server.on("/setAuto", HTTP_GET, handleAuto);
    server.on("/readMode", HTTP_GET, updateMode);
    server.on("/setFollow", HTTP_GET, handleFollow);

    // Tell server what to do in case of an error
    server.onNotFound(handleNotFound);

    // Serve local files to server
    server.serveStatic("/main.css", SPIFFS, "/main.css");
    server.serveStatic("/jquery.min.js", SPIFFS, "/jquery.min.js");

    // Handle events and send HTTP post code on success
    server.on("/forward", []() {
        driveForward();
        server.send(204);
    });

    server.on("/left", []() {
        turnDir(LEFT,300);
        server.send(204);
    });

    server.on("/right",[]() {
        turnDir(RIGHT,300);
        server.send(204);
     });
    
    server.on("/backward", []() {
        driveBackward();
        server.send(204);
    });

    server.on("/stop",[]() {
        handBrake();
        server.send(204);
    });

    // Start Server
    server.begin();

    Serial.println("HTTP server started");
}

void loop() {
    // Make robot aware of its current direction
    readDirection();
    
    // Handle server
    server.handleClient();

    // Force different states
    switch(d_State.mode) {
        case IDLE:
            break;
        case AUTO:
            collisionHandling();
            break;
        case FOLLOW:
            searchHand();
            break;
        default:
            Serial.println("Unregistered Mode!");
        break;
    }
}

#include <Arduino.h>
#include <Servo.h>
#include <SR04.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "edubot.h"

#define DEFAULT_SPEED 512
#define SERVO_DEFAULT 90
#define MINIMUM_SPEED 200
#define MAX_SPEED 1023
#define ROTATION_ANGLE 20
#define HOST "edubot"
#define PASSWORD "12345678"
#define DEBUG_LEVEL 2

Servo servo1;
SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);

drivingState_t dState = {
    FORWARD,
    DEFAULT_SPEED,
    DEFAULT_SPEED,
    IDLE
};

int currentSpeed = 0; // Make a variable to store the current speed
int debugLevel = DEBUG_LEVEL;
float a;


ESP8266WebServer server(80); // Start HTTP server at port 80

void setMode(drivingMode_t alteredMode) {
    if(alteredMode==dState.mode)
        dState.mode = IDLE;
    else 
        dState.mode = alteredMode;
}

void setDefaultSpeed() {
    analogWrite(MOTOR_A_SPEED, DEFAULT_SPEED);
    analogWrite(MOTOR_B_SPEED, DEFAULT_SPEED);
}

// Read ultrasonic sensor in cm and print log
void getDistance() {
    a=sr04.Distance();
    Serial.print(a);
    Serial.println("cm");
    delay(200);
}

void driveForward() {
    digitalWrite(MOTOR_A_ENABLE1, LOW);
    digitalWrite(MOTOR_A_ENABLE2, HIGH);
    digitalWrite(MOTOR_B_ENABLE1, LOW);
    digitalWrite(MOTOR_B_ENABLE2, HIGH);

    if(debugLevel > 1) {
    Serial.println("Driving forward");
    }
}

void driveBackward() {
    digitalWrite(MOTOR_A_ENABLE1, HIGH);
    digitalWrite(MOTOR_A_ENABLE2, LOW);
    digitalWrite(MOTOR_B_ENABLE1, HIGH);
    digitalWrite(MOTOR_B_ENABLE2, LOW);

    if(debugLevel > 1) {
    Serial.println("Driving backwards");
    }
}

// Invert direction, default is wheel driving forward
void changeDirA() {
    digitalWrite(MOTOR_A_ENABLE1, !digitalRead(MOTOR_A_ENABLE1));
    digitalWrite(MOTOR_A_ENABLE2, !digitalRead(MOTOR_A_ENABLE2));
}

void changeDirB() {
    digitalWrite(MOTOR_B_ENABLE1, !digitalRead(MOTOR_B_ENABLE1));
    digitalWrite(MOTOR_B_ENABLE2, !digitalRead(MOTOR_B_ENABLE2));
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

void stopAll() {
    // Set both motor pins on HIGH
    stopWheel(true);
    stopWheel(false);
    if( debugLevel > 1) {
    Serial.println("Robot is stopping...");
    }
}

void turnDir(direction_t dir, int time) {

    driveForward();

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
    // Perform circa 180 degree right turn 
    turnDir(RIGHT,500);
    stopAll();
}

void turnLeft() {
    // Perform circa 180 degree right turn 
    turnDir(LEFT,500);
    stopAll();
}

void turnServo() {
    // Start rotation from the middle
    int pos = SERVO_DEFAULT;

    // Rotate until rotation limit is reached
    for(pos=(SERVO_DEFAULT-ROTATION_ANGLE); pos<=(SERVO_DEFAULT+ROTATION_ANGLE); pos++) {
        servo1.write(pos);  
        delay(15);
      }
    // If limit is reached, count backwards
    for(pos=(SERVO_DEFAULT+ROTATION_ANGLE); pos>=(SERVO_DEFAULT-ROTATION_ANGLE); pos--) {
        servo1.write(pos);
        delay(15);
      }
      // Wait 1s between turns
      delay(1000);
}

void initServo() {
    // Set servo in a middle position
    servo1.write(SERVO_DEFAULT);
}

// Obstacle avoidance mode
void collisionHandling() {
    float distance = sr04.Distance();
    driveForward();
    // If ultrasonic distance is less than 10 perform a obstacle avoidance routine, else proceed driving
    if (distance > 0.0) {
        if (distance <= 10.0) {
            // Wait if the sensor value stabilizes
            stopAll();
            delay(500);

            do {
                    setDefaultSpeed();
                    delay(40);
                    turnDir(RIGHT,1000); 
                    distance = sr04.Distance(); // Update distance
            } 
            while (distance < 20.0);
            if( debugLevel > 1) {
                Serial.println("Avoided obstacle!");
            }
            stopAll();
            delay(500);
        }
   
        else if (distance < 20.0) {
            analogWrite(MOTOR_A_SPEED, DEFAULT_SPEED);
            analogWrite(MOTOR_B_SPEED, DEFAULT_SPEED);
            driveForward();
        }

        else {
            analogWrite(MOTOR_A_SPEED, MAX_SPEED);
            analogWrite(MOTOR_B_SPEED, MAX_SPEED);
            driveForward();
        }
    }
    delay(40);
}

// Drive an increasingly steeper circle
void driveSpiral() {
    Serial.println("--------------------");
    Serial.println("Driving a spiral");
    Serial.println("--------------------");

    if (dState.speedB > MINIMUM_SPEED) {
        dState.speedB--;                                // Decrementing motor speed on one side for an increasingly sharper curvature
        analogWrite(MOTOR_A_SPEED, dState.speedA);
        analogWrite(MOTOR_A_SPEED, dState.speedB);
        delay(150);                                     // Wait a little so the curve is not too sharp
    }
}


String prepareHtmlPage() {
        // Init string
        String htmlPage;

        // Open file
        File f = SPIFFS.open("/edubot.html","r");
        // Prevent error
    
        if (!f) {
            if(debugLevel > 1) {
            Serial.println("Error reading .html file!");
            }
        }
    
        // Read file into string
        else {
            htmlPage = f.readString();
            if( debugLevel > 1) {
            Serial.println("Reading files succesfully!");
            }
        }
        f.close();  // Close file
        return htmlPage;
}

void handleGet(){
    if(server.args()>0) {
        if(server.hasArg("speed")) {
            // Log current speed
            Serial.println(server.arg("speed").toInt());

            // Convert text to integer and set motor speed accordingly
            currentSpeed = abs(server.arg("speed").toInt());
            analogWrite(MOTOR_A_SPEED, currentSpeed);
            analogWrite(MOTOR_B_SPEED, currentSpeed); 
        }
    }
    else {
        // Send files to server
        server.send(200, "text/html", prepareHtmlPage()); // Push HTML code
    }
}

void handleNotFound() {
    // Print error in case something goes wrong
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

void setup() {
    // Initialize serial port
    Serial.begin(115200);

    // Start file system
    SPIFFS.begin();

     // Initialize servo pin
    servo1.attach(12);

     // Set all motor pins as outout
    pinMode(MOTOR_A_SPEED, OUTPUT);
    pinMode(MOTOR_A_ENABLE1, OUTPUT);
    pinMode(MOTOR_A_ENABLE2, OUTPUT);
    
    pinMode(MOTOR_B_SPEED, OUTPUT); 
    pinMode(MOTOR_B_ENABLE1, OUTPUT);
    pinMode(MOTOR_B_ENABLE2, OUTPUT);
    
    // Use access point mode
    WiFi.mode(WIFI_AP);

    // Define its name and password
    WiFi.softAP(HOST, PASSWORD);
    if (debugLevel > 1) {
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

    server.serveStatic("/main.css", SPIFFS, "/main.css");
    server.serveStatic("/jquery.min.js", SPIFFS, "/jquery.min.js");

    // Handle controls
    server.on("/forward", []() {
        driveForward();
        server.send(204);
    });

    server.on("/stop",[]() {
        stopAll();
        server.send(204);
    });

    server.on("/auto", []() {
        setMode(AUTO);
        server.send(204);
    });

    server.on("/left", []() {
        turnLeft();
        server.send(204);
    });

    server.on("/right",[]() {
        turnRight();
        server.send(204);
     });

    server.on("/spiral",[]() {
        setMode(SPIRAL);
        server.send(204);
    });

    server.on("/backwards", []() {
        driveBackward();
        server.send(204);
    });

    server.onNotFound(handleNotFound);

    // Start Server
    server.begin();
    Serial.println("HTTP server started");
}

void loop() { 
    // Handle server
    server.handleClient();
    // Force different states for continous execution
    switch(dState.mode){
        case IDLE:
            break;
        case AUTO:
            collisionHandling();
            break;
        case SPIRAL:
            driveSpiral();
            break;
        default:
        if( debugLevel > 1) {
            Serial.println("Unregistered Mode!");
        }
        break;
    }
} 

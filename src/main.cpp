#include <Arduino.h>
#include <Servo.h>
#include <SR04.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoOTA.h>
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
    NONE,
    0,
    0,
    IDLE
};

int debugLevel = DEBUG_LEVEL;

int currentSpeed = 0; // Make a variable to store the current speed

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
    delay(50);
}

void driveWheels(int valRight, int valLeft) {
    if (valLeft < 0) {
        digitalWrite(MOTOR_A_ENABLE1, HIGH);
        digitalWrite(MOTOR_A_ENABLE2, LOW);
        dState.speedA = valLeft;
    } else {
        digitalWrite(MOTOR_A_ENABLE1, LOW);
        digitalWrite(MOTOR_A_ENABLE2, HIGH);
        dState.speedA = valLeft;
    }
    if (valRight < 0) {
        digitalWrite(MOTOR_B_ENABLE1, HIGH);
        digitalWrite(MOTOR_B_ENABLE2, LOW);
        dState.speedB = valRight;
    } else {
        digitalWrite(MOTOR_B_ENABLE1, LOW);
        digitalWrite(MOTOR_B_ENABLE2, HIGH);
        dState.speedB = valRight;
    }

    if (valLeft == 0) {
        digitalWrite(MOTOR_A_ENABLE1, HIGH);
        digitalWrite(MOTOR_A_ENABLE2, HIGH);
        dState.speedB  = 0;
        
    }
    if (valRight == 0) {
        digitalWrite(MOTOR_B_ENABLE1, HIGH);
        digitalWrite(MOTOR_B_ENABLE2, HIGH);
        dState.speedB = 0;
    }
    analogWrite(MOTOR_A_SPEED, abs(valLeft));
    analogWrite(MOTOR_B_SPEED, abs(valRight));

}
 
void readDirection() {
    if (dState.speedA > 0 && dState.speedB > 0) {
        dState.dir = FORWARD;
    }
    else if (dState.speedA < 0  && dState.speedB < 0) {
        dState.dir = BACKWARD;
    }
    else if (dState.speedA > 0 && dState.speedB < 0) {
        dState.dir = RIGHT;
    }
    else if (dState.speedA < 0 && dState.speedB > 0) {
        dState.dir = LEFT;
    }
    else {
        dState.dir = NONE;
    }
}

void driveForward() {
    driveWheels(dState.speedA,dState.speedB);

    if(debugLevel > 1) {
        Serial.println("Driving forward");
    }
}

void driveBackward() {
    driveWheels(-dState.speedA,-dState.speedB);

    if(debugLevel > 1) {
        Serial.println("Driving backwards");
    }
}

// Change direction, default is wheel driving forward
void changeDirA() {
    driveWheels(dState.speedA,-dState.speedB);
}

void changeDirB() {
     driveWheels(-dState.speedA,dState.speedB);
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
    if(dState.dir == FORWARD) {
        turnDir(RIGHT,250);
        handBrake();
        delay(200);
        driveForward();
    }
    else if(dState.dir == BACKWARD) {
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
    if(dState.dir == FORWARD) {
        turnDir(LEFT,250);
        handBrake();
        delay(200);
        driveForward();
    }
    else if(dState.dir == BACKWARD) {
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
    setDefaultSpeed();
    // If ultrasonic distance is less than 10 perform a obstacle avoidance routine, else proceed driving
    if (distance > 0.0) {
        if (distance <= 10.0) {
            // Wait if the sensor value stabilizes
            handBrake();
            delay(500);
            setDefaultSpeed();
            do {
                    delay(40);
                    turnDir(RIGHT, 500);
                    distance = sr04.Distance(); // Update distance
            } 
            while (distance < 20.0);
            if( debugLevel > 1) {
                Serial.println("Avoided obstacle!");
            }
            handBrake();
            delay(500);
            driveForward();
        } else if (distance > 20.0) {
            analogWrite(MOTOR_A_SPEED, DEFAULT_SPEED);
            analogWrite(MOTOR_B_SPEED, DEFAULT_SPEED);
            driveForward();
        } else {
            analogWrite(MOTOR_A_SPEED, MAX_SPEED);
            analogWrite(MOTOR_B_SPEED, MAX_SPEED);
            driveForward();
        }
    }
    delay(40);
}

// Make robot behave like a pet by following the owners hand
void searchHand() {

}


String prepareHtmlPage() {
        String htmlPage;                                // Init string
        File f = SPIFFS.open("/edubot.html","r");       // Open file
        // Check for an error
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
            if (debugLevel > 2){
                Serial.println(server.arg("speed").toInt());
            }
            // Convert text to integer and set motor speed accordingly
            dState.speedA = abs(server.arg("speed").toInt());
            dState.speedB = abs(server.arg("speed").toInt());
            analogWrite(MOTOR_A_SPEED, dState.speedA);
            analogWrite(MOTOR_B_SPEED, dState.speedB); 
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

    ArduinoOTA.setHostname("ESP8266");
    ArduinoOTA.setPassword("esp8266");

    if (debugLevel > 1) {
        ArduinoOTA.onStart([]() {
            Serial.println("Start");
        });

        ArduinoOTA.onEnd([]() {
            Serial.println("\nEnd");
        });

        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        });

        ArduinoOTA.onError([](ota_error_t error) {
                Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });
    }

    ArduinoOTA.begin();

    if (debugLevel > 1) {
    Serial.println("OTA ready");
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
        handBrake();
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

    server.on("/follow",[]() {
        setMode(FOLLOW);
        server.send(204);
    });

    server.on("/backwards", []() {
        driveBackward();
        server.send(204);
    });

    server.onNotFound(handleNotFound);

    // Start Server
    server.begin();
    if(debugLevel > 1) {
        Serial.println("HTTP server started");
    }
}

void loop() {
    // Handle remote uploading
    ArduinoOTA.handle();

    readDirection();
    driveWheels(dState.speedA, dState.speedB);
    // Handle server
    server.handleClient();

    // Force different states for continous execution
    switch(dState.mode){
        case IDLE:
            break;
        case AUTO:
            collisionHandling();
            break;
        case FOLLOW:
            searchHand();
            break;
        default:
        if( debugLevel > 1) {
            Serial.println("Unregistered Mode!");
        }
        break;
    }
} 

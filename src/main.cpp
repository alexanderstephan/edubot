#include <Arduino.h>
#include <Servo.h>
#include <SR04.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "edubot.h"
#include <motor.h> 

#define DEFAULT_SPEED 512
#define SERVO_DEFAULT 90
#define SERVO_RIGHT 45
#define SERVO_LEFT 135
#define MINIMUM_SPEED 200
#define MAX_SPEED 1023
#define ROTATION_ANGLE 20
#define HOST "edubot"
#define PASSWORD "12345678"
#define HAND_DISTANCE 15
#define TOO_CLOSE 4

#define DEBUG_LEVEL 2
int debug_Level = DEBUG_LEVEL;
int currentSpeed = 0; // Make a variable to store the current speed
float distance;

Servo servo1;
SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);

drivingState_t d_State = {
    NONE,
    0,
    0,
    IDLE
};

ESP8266WebServer server(80); // Start HTTP server at port 80

void setMode(drivingMode_t alteredMode) {
    if(alteredMode == d_State.mode) {
        d_State.mode = IDLE;
        }
    else {
        d_State.mode = alteredMode;
    }
}

// Read ultrasonic sensor in cm and print log
void getDistance() {
    distance=sr04.Distance();
    Serial.print(distance);
    Serial.println("cm");
    delay(50);
}
 
void turnServo(int servoDegree) {
    // Start rotation from the middle
    int pos = SERVO_DEFAULT;
    int turnDegree = servoDegree;

    // Rotate until rotation limit is reached
    for(pos=(SERVO_DEFAULT-servoDegree); pos<=(SERVO_DEFAULT+servoDegree); pos++) {
        servo1.write(pos);  
        delay(15);
      }
    // If limit is reached, count backwards
    for(pos=(SERVO_DEFAULT-turnDegree); pos>=(SERVO_DEFAULT-turnDegree); pos--) {
        servo1.write(pos);
        delay(15);
      }
}

void initServo(int servoPos) {
    servo1.write(servoPos);    // Set servo in a middle position
}

// Obstacle avoidance mode
void collisionHandling() {
    distance = sr04.Distance();
    driveForward();
    turnServo(20);
    // If ultrasonic distance is less than 10 perform a obstacle avoidance routine, else proceed driving
    if (distance > 0.0) {
        if (distance <= 10.0) {     
            // Wait if the sensor value stabilizes
            handBrake();   
            delay(500);                                      
            do {
                    delay(40);
                    turnDir(RIGHT, 600);            // Turn right until the object is out of sight
                    distance = sr04.Distance();     // Update distance
            } 
            while (distance < 20.0);

            if( debug_Level > 1) {
                Serial.println("Avoided obstacle!");
            }
            // If threshold is reached wait a little to to see if the obstacle distance stays above 10cm
            handBrake();
            turnServo(20);                                   
            delay(500);
            driveForward(); // Continue driving forward
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
int searchHand() {
    distance = sr04.Distance();
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
            distance = sr04.Distance(); 

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

            distance = sr04.Distance(); 
        
            if (distance <= HAND_DISTANCE) {
                handPosition = servoPosition;
                return handPosition;
            }
        }
    } while (distance > HAND_DISTANCE); // Search while hand is not within reach
}

void followHand() {
    initServo(SERVO_DEFAULT);
    analogWrite(MOTOR_A_SPEED, MAX_SPEED);
    analogWrite(MOTOR_B_SPEED, MAX_SPEED);
    driveForward();

    do {
        distance = sr04.Distance();
        delay(10);
    } while ((distance <= HAND_DISTANCE) && (distance <= TOO_CLOSE));
    handBrake();
    delay(1000);

}

void turnTowardsHand() {
    
}
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

void handleGet(){
    if(server.args()>0) {
        if(server.hasArg("speed")) {
            if (debug_Level > 2){
                Serial.println(server.arg("speed").toInt());      // Log current speed
            }
            /* Convert text to integer and set motor speed accordingly */
            d_State.speedA = server.arg("speed").toInt();
            d_State.speedB = server.arg("speed").toInt();
            analogWrite(MOTOR_A_SPEED, abs(d_State.speedA));
            analogWrite(MOTOR_B_SPEED, abs(d_State.speedB)); 
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

    init(&d_State);

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
    if(debug_Level > 1) {
        Serial.println("HTTP server started");
    }
}

void loop() {
    // Make robot aware of its current direction
    readDirection();

    // Handle server
    server.handleClient();

    // Force different states for continous execution
    switch(d_State.mode){
        case IDLE:
            break;
        case AUTO:
            collisionHandling();
            break;
        case FOLLOW:
            searchHand();
            break;
        default:
        if( debug_Level > 1) {
            Serial.println("Unregistered Mode!");
        }
        break;
    }
} 

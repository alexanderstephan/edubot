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

Servo servo1;

SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
float a;

bool autoEnabled=false;

int speedA = DEFAULT_SPEED;
int speedB = DEFAULT_SPEED;
int currentSpeed = 0; // Make a variable to store the current speed

ESP8266WebServer server(80); // Start HTTP server at port 80

void setAuto(){
	autoEnabled = !autoEnabled;
    stopAll(); // Signaling mode change
    setDefaultSpeed(); // Make sure roboto isn't moving with maximum speed
    delay(300);
}

void setDefaultSpeed(){
    analogWrite(MOTOR_A_SPEED, DEFAULT_SPEED);
    analogWrite(MOTOR_B_SPEED, DEFAULT_SPEED);
}

// Read ultrasonic sensor in cm and print log
void getDistance(){
    a=sr04.Distance();
    Serial.print(a);
    Serial.println("cm");
    delay(200);
}

void driveForward(){
    Serial.println("--------------------");
    Serial.println("Driving forward");
    Serial.println("--------------------");

    digitalWrite(MOTOR_A_ENABLE1, LOW);
    digitalWrite(MOTOR_A_ENABLE2, HIGH);
    digitalWrite(MOTOR_B_ENABLE1, LOW);
    digitalWrite(MOTOR_B_ENABLE2, HIGH);
    server.send(204);
}

void driveBackward(){
    Serial.println("--------------------");
    Serial.println("Driving backwards");
    Serial.println("--------------------");
    digitalWrite(MOTOR_A_ENABLE1, HIGH);
    digitalWrite(MOTOR_A_ENABLE2, LOW);
    digitalWrite(MOTOR_B_ENABLE1, HIGH);
    digitalWrite(MOTOR_B_ENABLE2, LOW);
    server.send(204);
}

// Invert direction, default is wheel driving forward
void changeDirA(){
    digitalWrite(MOTOR_A_ENABLE1, !digitalRead(MOTOR_A_ENABLE1));
    digitalWrite(MOTOR_A_ENABLE2, !digitalRead(MOTOR_A_ENABLE2));
}

void changeDirB(){
    digitalWrite(MOTOR_B_ENABLE1, !digitalRead(MOTOR_B_ENABLE1));
    digitalWrite(MOTOR_B_ENABLE2, !digitalRead(MOTOR_B_ENABLE2));
}

void stopWheel(bool left){
    if(left){
        digitalWrite(MOTOR_A_ENABLE1, HIGH);
        digitalWrite(MOTOR_A_ENABLE2, HIGH);
    }
    else{
        digitalWrite(MOTOR_B_ENABLE1, HIGH);
        digitalWrite(MOTOR_B_ENABLE2, HIGH);
    }
}

void stopAll(){
    Serial.println("--------------------");
    Serial.println("Robot is stopping...");
    Serial.println("--------------------");

    // Set both motor pins on HIGH
    stopWheel(true);
    stopWheel(false);

    //server.sendHeader("Location", "/");
    server.send(204);

}

void turnDir(direction_t dir, int time){
    // Read both motor speeds
    speedA = analogRead(MOTOR_A_SPEED);
    speedB = analogRead(MOTOR_B_SPEED);

    // Make sure robot is driving forward
    driveForward();

    // If function argument equals LEFT, perform a right turn
    Serial.println("--------------------");
    if(dir==LEFT){
        Serial.println("Turning left");
        changeDirA();
        delay(time);
    }
    // If function argument is not LEFT, perform a right turn
    else if(dir==RIGHT){
        Serial.println("Turning right");
        changeDirB();
        delay(time);
    }
    else{
        Serial.println("Error reading direction!");
    }

    Serial.println("--------------------");
}

void turnRight(){
    // Perform circa 180 degree right turn 
    turnDir(RIGHT,500);
    stopAll();
}

void turnLeft(){
    // Perform circa 180 degree right turn 
    turnDir(LEFT,500);
    stopAll();
}

void turnServo(){
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

void initServo(){
    // Set servo in a middle position
    servo1.write(SERVO_DEFAULT);
}

// Obstacle avoidance mode
void collisionHandling(){
    float distance = sr04.Distance();

    // If ultrasonic distance is less than 10 perform a obstacle avoidance routine, else proceed driving
    if (distance > 0.0){
        if (distance <= 10.0)
        {
            // Wait if the sensor value stabilizes
            stopAll();
            delay(500);
            // Perform a turning action until object is out of sight

            do {
                distance = sr04.Distance();
                turnDir(RIGHT,100);
                delay(40);
            } 
            while (distance < 20.0);

            //turnServo(); // Make sure angle isn't tricking the sensor
            delay(500);
            //initServo();

            // Make speed lower to give more accurate ultra sonic sensor measurements
            analogWrite(MOTOR_A_SPEED, MINIMUM_SPEED); 
            analogWrite(MOTOR_B_SPEED, MINIMUM_SPEED);
            driveForward();
        }
   
        else if (distance < 30.0){
            analogWrite(MOTOR_A_SPEED, DEFAULT_SPEED);
            analogWrite(MOTOR_B_SPEED, DEFAULT_SPEED);
            driveForward();
        }

        else{
            analogWrite(MOTOR_A_SPEED, MAX_SPEED);
            analogWrite(MOTOR_B_SPEED, MAX_SPEED);
            driveForward();
        }
    }
    delay(40);
}

// Drive an increasingly steeper circle
void driveSpiral(){
    Serial.println("--------------------");
    Serial.println("Driving a spiral");
    Serial.println("--------------------");

    analogWrite(MOTOR_A_SPEED, DEFAULT_SPEED);
    analogWrite(MOTOR_B_SPEED, DEFAULT_SPEED);
    if (speedB > MINIMUM_SPEED) {
        speedB--; // Decrementing motor speed on one side for an increasingly sharper curvature
        analogWrite(MOTOR_A_SPEED, speedA);
        analogWrite(MOTOR_A_SPEED, speedB);
        delay(150); // Wait a little so the curve is not too sharp
    }
    server.send(204);
}

// Fill HTML page in a string that will be sent to the server
String prepareHtmlPage(){  
    String htmlPage;
    File f = SPIFFS.open("/edubot.html","r");
    if (!f) {
        Serial.println("Error reading file!");
    } else {
        htmlPage = f.readString();
        Serial.println("Reading file succesfully!");
    }
    f.close();
    return htmlPage;
}

void handleGet(){
    if(server.args()>0){
        if(server.hasArg("speed")){
            // Log current speed
            Serial.println(server.arg("speed").toInt());

            // Convert text to integer and set motor speed accordingly
            currentSpeed = abs(server.arg("speed").toInt());
            analogWrite(MOTOR_A_SPEED, currentSpeed);
            analogWrite(MOTOR_B_SPEED, currentSpeed); 

            /*
            
            if(server.arg("speed").toInt()<0){ // If the value is lower than zero then drive backwards
                driveBackward();
            }
            else{
                driveForward();
            }
            */
        }
    }
    else{
        server.send(200, "text/html", prepareHtmlPage()); // Push HTML code
    }
}

void handleNotFound(){
    // Print error in case something goes wrong
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for (uint8_t i=0; i<server.args(); i++){
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }

    server.send(404, "text/plain", message);
}

void setup(){
    // Initialize serial port
    Serial.begin(115200);

    // Start file system
    SPIFFS.begin();

     // Initialize servo pin
    servo1.attach(12);

     // Set all Motor Pins as outout
    pinMode(MOTOR_A_SPEED, OUTPUT);
    pinMode(MOTOR_A_ENABLE1, OUTPUT);
    pinMode(MOTOR_A_ENABLE2, OUTPUT);
    
    pinMode(MOTOR_B_SPEED, OUTPUT); 
    pinMode(MOTOR_B_ENABLE1, OUTPUT);
    pinMode(MOTOR_B_ENABLE2, OUTPUT);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(HOST, PASSWORD);
    Serial.println("");

    // Print IP adress
    Serial.println("");
    Serial.print("Name of Access Point is:");
    Serial.println("edubot");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    
    // Handle GET requests
    server.on("/",HTTP_GET,handleGet);
    server.on("/forward",driveForward);
    server.on("/stop",stopAll);
    server.on("/auto",setAuto); 
    server.on("/left", turnLeft);
    server.on("/right",turnRight);
    server.on("/spiral",driveSpiral);
    server.on("/backwards", driveBackward);
    server.onNotFound(handleNotFound);

    // Start Server
    server.begin();
    Serial.println("HTTP server started");
}

void loop(){ 
    //server.handleClient();
    if(autoEnabled){
        server.handleClient();
        collisionHandling();
    }
    else if(!autoEnabled){ 
        server.handleClient(); // Handle requests
    }
} 

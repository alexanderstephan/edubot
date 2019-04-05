#include <Arduino.h>
#include <Servo.h>
#include <SR04.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
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
long a;

int speedA = DEFAULT_SPEED;
int speedB = DEFAULT_SPEED;
int currentSpeed = 0; // Make a variable to store the current speed

ESP8266WebServer server(80); // Start HTTP server at port 80

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
}

void driveBackward(){
    Serial.println("--------------------");
    Serial.println("Driving backwards");
    Serial.println("--------------------");
    digitalWrite(MOTOR_A_ENABLE1, HIGH);
    digitalWrite(MOTOR_A_ENABLE2, LOW);
    digitalWrite(MOTOR_B_ENABLE1, HIGH);
    digitalWrite(MOTOR_B_ENABLE2, LOW);
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

    return;
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
    return;
}

void turnLeft(){
    // Perform circa 180 degree right turn 
    turnDir(LEFT,500);
    stopAll();
    return;
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
    Serial.println("--------------------");
    Serial.println("Entering collison handling mode");
    Serial.println("--------------------");

    // If ultrasonic distance is less than 10 perform a obstacle avoidance routine, else proceed driving
    if(sr04.Distance() < 10){   
        driveBackward();
        delay(1000);
        stopAll();
        delay(500);
        turnRight();
        delay(500);
        driveForward();
        delay(500);
        turnServo();
        }
    else{
        driveForward();
    }
}

// Drive an increasingly steeper circle
void driveSpiral() {
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
}

// Fill HTML page in a string that will be sent to the server
String prepareHtmlPage(){  
	String htmlPage = String("")+
    "<!DOCTYPE HTML>"+
    "<html>"+
    "<head>"+
    "<meta charset=\"utf-8\"/>"+
    "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">	<style>	@import url('https://fonts.googleapis.com/css?family=Roboto');	html {font-family: 'roboto', sans-serif; text-transform: uppercase;}	body {background-color: #fdd835}	button{ background-color: #4CAF50; border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin-left: 10px; margin-right: 10px; margin-top: 20px; margin-bottom: 20px;}	</style></head>"+
    "<body>"+
    "<center>"+
    "<h1 style=\"color:white; font-size: 40px\";>Edubot</h1>"+
    "<a href=\"auto\"><button>Auto-Mode</button></a><p>"+
    "<a href=\"stop\"><button>Stop Engine</button></a><br>"+
    "<h2 style=\"color:white\">Velocity:</h2><input type=\"range\" min=\"-1024\" max=\"1024\" value=\"50\" class=\"slider\" id=\"myRange\" onchange=\"speedChanged(this.value)\"><p>"+
    "<a href=\"forward\"><button>Drive</button></a><br>"+
    "<a href=\"left\"><button>Left</button></a>"+
    "<a href=\"right\"><button>Right</button></a><br>"+
    "<a href=\"backwards\"><button>Drive Backwards</button></a><br>"+
    "<a href=\"spiral\"><button>Drive Spiral</button></a><br>"+
    "</center>"+
    "</body>"+
    "<script>"+
    "$.ajaxSetup({timeout:1000});"+
    "function speedChanged(speed) {	$.get(\"?speed=\" + speed );}"+
    "</script>"+
    "</html>";
    return htmlPage;
}

void handleGet(){
    if(server.args()>0){ // Check wether there is an input
        if(server.hasArg("speed")){
            // Log current speed
            Serial.println(server.arg("speed"));

            // Convert text to integer and set motor speed accordingly
            currentSpeed = abs(server.arg("speed").toInt());
            analogWrite(MOTOR_A_SPEED, currentSpeed);
            analogWrite(MOTOR_B_SPEED, currentSpeed); 
            
            if(server.arg("speed").toInt()<0){ // If the value is lower than zero then drive backwards
                driveBackward();
                /* Make variable to store the current direction*/
            }
            else{
                driveForward();
            }
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

     // Initialize servo pin
    servo1.attach(12);

     // Set all Motor Pins as outout
    pinMode(MOTOR_A_SPEED, OUTPUT);
    pinMode(MOTOR_A_ENABLE1, OUTPUT);
    pinMode(MOTOR_A_ENABLE2, OUTPUT);
    
    pinMode(MOTOR_B_SPEED, OUTPUT); 
    pinMode(MOTOR_B_ENABLE1, OUTPUT);
    pinMode(MOTOR_B_ENABLE2, OUTPUT);
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
    server.on("/auto",collisionHandling); 
    server.on("/left", turnLeft);
    server.on("/right",turnRight);
    server.on("/spiral",driveSpiral);
    server.on("/backwards", driveBackward);
    server.onNotFound(handleNotFound);

    analogWrite(MOTOR_A_SPEED, MAX_SPEED);
    analogWrite(MOTOR_B_SPEED, MAX_SPEED);

    // Start Server
    server.begin();
    Serial.println("HTTP server started");
}

void loop() { 
    server.handleClient(); // Handle requests
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

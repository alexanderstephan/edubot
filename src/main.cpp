#include <Arduino.h>
#include <Servo.h>
#include <SR04.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "edubot.h"

#define MOTOR_A_ENABLE1 16 
#define MOTOR_A_ENABLE2 14
#define MOTOR_A_SPEED 15

#define MOTOR_B_ENABLE1 0                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
#define MOTOR_B_ENABLE2 2
#define MOTOR_B_SPEED 12

#define SERVO_PWM 13

#define ECHO_PIN 5 
#define TRIG_PIN 4

#define defaultSpeed 512

Servo servo1;

SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
long a;

String request;

int minimumSpeed = 200;

int speedA = defaultSpeed;
int speedB = defaultSpeed;
int alteredSpeed = 0;


ESP8266WebServer server(80);

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
    analogWrite(MOTOR_A_SPEED, alteredSpeed);
    analogWrite(MOTOR_B_SPEED, alteredSpeed);
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
    stopWheel(true);
    stopWheel(false);
    
}
/*
void turnDegree(direction_t dir, int degree){ //todo

}

void turnTime(direction_t dir, int time){
    driveForward();
    Serial.println("--------------------");
    if(dir==LEFT){
        Serial.println("Turning left");
        changeDirA();
    }
    else{
        Serial.println("Turning right");
        changeDirB();
    }
    Serial.println("--------------------");
    delay(time);
}
*/

// deprecated please do not use; use turnTime instead
void turnLeft(){
    Serial.println("--------------------");
    Serial.println("Turning left");
    Serial.println("--------------------");
    driveForward();
    analogWrite(MOTOR_A_SPEED, defaultSpeed);
    analogWrite(MOTOR_B_SPEED, defaultSpeed);
    changeDirA();
    delay(800);
}

//deprecated please do not use; use turnTime instead
void turnRight(){
    Serial.println("--------------------");
    Serial.println("Turning right");
    Serial.println("--------------------");
    speedA = analogRead(MOTOR_A_SPEED);
    speedB = analogRead(MOTOR_B_SPEED);
    driveForward();
    analogWrite(MOTOR_A_SPEED, 512);
    analogWrite(MOTOR_B_SPEED, 512);
    changeDirB();
    delay(800);
  
}

void turnServo(){
    int pos=90;
    for(pos=60; pos<=120; pos++) {
        servo1.write(pos);  
        delay(100);   
      }

    for(pos=120; pos>=60; pos--) {
        servo1.write(pos);
        delay(100);
      }
    delay(500);
}

void stopServo(){
    servo1.write(90);
}

void collisionHandling(){
    Serial.println("--------------------");
    Serial.println("Entering collison handling mode");
    Serial.println("--------------------");
    if(sr04.Distance() < 10){   
        driveBackward();
        delay(2000);
        stopAll();
        delay(500);
        driveForward();
        changeDirA();
        delay(2000);
        driveForward();
        stopServo();
        }
    else{
        driveForward();
        turnServo();
    }
}

void driveSpiral() {
    Serial.println("--------------------");
    Serial.println("Driving a spiral");
    Serial.println("--------------------");
    analogWrite(MOTOR_A_SPEED, defaultSpeed);
    analogWrite(MOTOR_B_SPEED, defaultSpeed);
    if (speedB > minimumSpeed) {
        speedB--; // Decrementing motor speed on one side for an increasingly sharper curvature
        analogWrite(MOTOR_A_SPEED, speedA);
        analogWrite(MOTOR_A_SPEED, speedB);
        delay(150); // Wait a little so the curve is not too sharp
    }
}

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
    "<a href=\"forward\"><button>Forward</button></a><br>"+
    "<a href=\"left\"><button>Left</button></a>"+
    "<a href=\"right\"><button>Right</button></a><br>"+
    "<a href=\"spiral\"><button>Drive Spiral</button></a><br>"+
    "</center>"+
    "</body>"+
    "<script>	$.ajaxSetup({timeout:5000});	function speedChanged(speed) {	$.get(\"?speed=\" + speed );    }</script>"+
    "</html>";
    return htmlPage;
}

void handleGet(){
    if(server.args()>0){ //Check wether there is an input
        if(server.hasArg("speed")){
            //Log current speed
            Serial.println(server.arg("speed"));
            //Convert text to integer and set motor speed accordingly
            alteredSpeed = abs(server.arg("speed").toInt());
            analogWrite(MOTOR_A_SPEED, alteredSpeed);
            analogWrite(MOTOR_B_SPEED, alteredSpeed); 
            
            if(server.arg("speed").toInt()<0){ //If the value is lower than zero then drive backwards
                driveBackward();
            }
            else{
                driveForward();
            }
        }
    }
    else{
        server.send(200, "text/html", prepareHtmlPage());
    }
}

void handleNotFound(){
    //Print error in case something goes wrong
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
    //Initialize serial port
    Serial.begin(115200);

     // Initialize servo pin
    servo1.attach(13);

     //Set all Motor Pins as outout
    pinMode(MOTOR_A_ENABLE1, OUTPUT);
    pinMode(MOTOR_A_ENABLE2, OUTPUT);
    pinMode(MOTOR_A_SPEED, OUTPUT);
    pinMode(MOTOR_B_ENABLE1, OUTPUT);
    pinMode(MOTOR_B_ENABLE2, OUTPUT);
    pinMode(MOTOR_B_SPEED, OUTPUT); 

    // Set default motor speed to 512
    analogWrite(MOTOR_A_SPEED, defaultSpeed);
    analogWrite(MOTOR_B_SPEED, defaultSpeed);

    ///Initialize Access Point
    WiFi.mode(WIFI_AP); //Access Point mode
    WiFi.softAP("edubot", "12345678");
    Serial.println("");

    //Print IP adress
    Serial.println("");
    Serial.print("Name of Access Point is:");
    Serial.println("edubot");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    
    //Handle requests
    server.on("/",HTTP_GET,handleGet);
    server.on("/forward",driveForward);
    server.on("/stop",stopAll);
    server.on("/auto",collisionHandling); 
    server.on("/left",turnLeft);
    server.on("/right",turnRight);
    server.on("/spiral",driveSpiral);
    server.onNotFound(handleNotFound);

    //Start Server
    server.begin();
    Serial.println("HTTP server started");
}

void loop() { 
    server.handleClient(); // Handle requests
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

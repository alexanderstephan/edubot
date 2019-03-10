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
    digitalWrite(MOTOR_A_ENABLE1, LOW);
    digitalWrite(MOTOR_A_ENABLE2, HIGH);
    digitalWrite(MOTOR_B_ENABLE1, LOW);
    digitalWrite(MOTOR_B_ENABLE2, HIGH);
}

void changeDirA(){
    digitalWrite(MOTOR_A_ENABLE1, !digitalRead(MOTOR_A_ENABLE1));
    digitalWrite(MOTOR_A_ENABLE2, !digitalRead(MOTOR_A_ENABLE2));
}

void changeDirB(){
    digitalWrite(MOTOR_B_ENABLE1, !digitalRead(MOTOR_B_ENABLE1));
    digitalWrite(MOTOR_B_ENABLE2, !digitalRead(MOTOR_B_ENABLE2));
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


// deprecated please do not use; use turnTime instead
void turnLeft(){
    Serial.println("--------------------");
    Serial.println("Turning left");
    Serial.println("--------------------");
    driveForward();
    changeDirA();
    delay(800);
}

//deprecated please do not use; use turnTime instead
void turnRight(){
    Serial.println("--------------------");
    Serial.println("Turning right");
    Serial.println("--------------------");
    driveForward();
    changeDirB();
    delay(800);
}

void turnServo(){
    int pos=90;
    for(pos=60; pos<=120; pos++) {
        servo1.write(pos);     
      }

    for(pos=120; pos>=60; pos--) {
        servo1.write(pos);
      }
    delay(500);
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
        }
    else{
        driveForward();
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
	String htmlPage =
		String("")+
            "<html>" +
            "<head>\n"+
            "\t<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>\n"+
            "</head>"+
            "<center>"+
            "<h1>Edubot Early Access</h1>"+
            "<a href=\"/start\"><button>Start Engine</button></a>\t<a href=\"/stop\"><button>Stop Engine</button></a>\t  <a href=\"/auto\"><button>Auto-Mode</button></a><p>"+ //start, stop and auto button
            "Velocity:<input type=\"range\" min=\"-1024\" max=\"1024\" value=\"50\" class=\"slider\" id=\"myRange\" onchange=\"speedChanged(this.value)\"><p>"+ //Change speed
            "<a href=\"/left\"><button>Rotate 90 degrees left</button></a>\t<a href=\"/right\"><button>Rotate 90 degrees right</button></a>"+ //Change direction
            "\t<a href=\"/spiral\"><button>Drive Spiral</button></a>\t"+
            "</center>"+
            "<script>"+
                "$.ajaxSetup({timeout:1000});\n"+
                "function speedChanged(speed) {\n"+
                "\t$.get(\"/?speed=\" + speed );\n"+
                "}"+
            "</script>"+
            "</html>" +
            "\r\n";
  return htmlPage;
}

void handleGet(){
    if(server.args()>0){ //Check wether there is an input
        if(server.hasArg("speed")){
            //Log current speed
            Serial.println(server.arg("speed"));
            //Convert text to integer and set motor speed accordingly
            analogWrite(MOTOR_A_SPEED, abs(server.arg("speed").toInt()));
            analogWrite(MOTOR_B_SPEED, abs(server.arg("speed").toInt())); 
            
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
    delay(500); 
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    
    //Handle requests
    server.on("/",HTTP_GET,handleGet);
    server.on("/start",driveForward);
    server.on("/stop",stopAll);
    server.on("/auto",collisionHandling); 
    server.on("/left",turnLeft);
    server.on("/right",turnRight);
    server.on("/spiral",driveSpiral);
    server.onNotFound(handleNotFound);

    //Start Server
    server.begin();
    Serial.println("HTTP server started");

    // Initialize servo pin
    servo1.attach(13);
}

void loop() { 
    server.handleClient(); // Handle requests
    getDistance();
    delay(100); 
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

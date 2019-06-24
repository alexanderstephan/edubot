#ifndef EDUBOT_EDUBOT_H
#define EDUBOT_EDUBOT_H

/* Map out NodeMCU pins for easier access 
   Already used pins are marked with '#'
Data pins:
    BUILTIN_LED -> 16
    D0 ->  16
    # D1 ->  5
    # D2 ->  4
    # D3 ->  0
    # D4 ->  2
    D5 ->  14
    D6 ->  12
    D7 ->  13
    D8 ->  15
    D9 ->  3
    D10 -> 1
*/

/* Servo pin */
#define SERVO_PWM 15

/* Sensor pins */
#define ECHO_PIN 12
#define TRIG_PIN 13
#define pinDHT11 16

/* Sensor functions */
void getDistance();

/* Robot movement */
void initServo();
void turnServo();
void collisionHandling();
void driveSpiral();
void setMode();

/* Server */
String prepareHtmlPage();
void handleGet();
void handleNotFound();
void updateMode();
void updateDistance();
void updateHumid();

#endif //EDUBOT_EDUBOT_H

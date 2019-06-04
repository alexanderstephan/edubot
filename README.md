# A Robot For Use In Education

### Table of Contents
1. [Overview](#Overview)
2. [How To Build](#how-to-build)
3. [Wiring](#Wiring)
4. [Excercises](#possible-excercises)
5. [To Do List](#to-do-list)

## Overview
I started the project as a project paper for my school. The acrylic chassis is from watterott.com and has a nice corresponding book from Markus Knapp called 'Building Robots With Arduino'. 
The robot has the basic ability to be controlled manually as well as an obstacle avoidance mode. Everything is controlled via a simple web page which can be accessed by connecting to the ESP8266s local network. 

![Robot](https://raw.githubusercontent.com/alexanderstephan/edubot/master/bot.jpg)

## How To Build

- `git clone https://github.com/alexanderstephan/edubot.git`
- Install [PlatformIO Core](https://docs.platformio.org/en/latest/core.html) i.e. via [AUR](https://aur.archlinux.org/packages/platformio-git/) on Arch Linux
- `cd edubot && platformio run` (You can specify upload_port in platform.ini)
- To access the control interface connect to the AP with the specified password and navigate your browser to APs IP probably `192.168.4.1`
## Wiring
![Schematic](https://raw.githubusercontent.com/alexanderstephan/edubot/master/edubot_bb.png)

## Possible excercises
- Make robot drive in a circle
- Make robot drive in zigzag pattern
- Make a basic obstacle recognition
- Implement simple motor logic

## To do list
- Add 100nF capacitors to both gear motors
- Add resistor to ultra sonic sensor
- Wheels do not have exactly the same speed

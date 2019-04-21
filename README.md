### EDUBOT
## A Robot For Use In Education

## Table of Contents
1. [Overview](#Overview)
2. [How To Build](#how-to-build)
3. [Wiring](#Wiring)

# Overview
I started the project as a project paper for my school. The acrylic chassis is from watterott.com and has a nice corresponding book from Markus Knapp called 'Building Robots With Arduino'. 
The robot has the basic ability to be controlled manually as well as an obstacle avoidance mode. Everything is controlled via a simple web page which can be accessed by connecting to the ESP8266s local network. 

![Robot](https://raw.githubusercontent.com/alexanderstephan/edubot/master/bot.jpg)

# How To Build

- `git clone https://github.com/alexanderstephan/edubot.git`
- Install [PlatformIO Core](https://docs.platformio.org/en/latest/core.html) i.e. via [AUR](https://aur.archlinux.org/packages/platformio-git/) on Arch Linux
- `cd edubot && platformio run` (You can specify upload_port in platform.ini)

# Wiring
![Schematic](https://raw.githubusercontent.com/alexanderstephan/edubot/master/edubot_bb.png)
# To do list
- ~~Adding a local jquery/AJAX file with SPLIFF~~
- ~~Fixing issue with random connection reset~~
- ~~Finishing the spiral routine~~
- Make "Auto-Mode" button show current mode
- Make turning movements smoother
- Add 100nF capacitors to both gear motors
- Add resistor to ultra sonic sensor

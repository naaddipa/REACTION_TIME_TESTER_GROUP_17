Reaction Time Tester – Group 17 (Computer Engineering)

Project Overview

The Reaction Time Tester is a microprocessor-based game designed to measure how quickly a player responds to a visual and auditory stimulus. 
When an LED turns on at a random time, a buzzer sounds to alert the player. The player must then press a button as quickly as possible. 
The system records the reaction time and displays it on an LCD screen.

How It Works

1. The system initializes and waits for a random delay.


2. After the delay, the LED turns on and the buzzer sounds to alert the player.


3. The player must press the button as fast as possible.


4. The Arduino records the reaction time from the moment the LED turns on to when the button is pressed.


5. The reaction time is displayed on the LCD screen.



Components Used

Microcontroller: Arduino (Uno/Nano)

LED: Indicator for reaction test (connected to D10)

Buzzer: Sounds when LED turns on (connected to D9)

Push Button: Player input (connected to D7)

LCD Display: Shows the reaction time (I2C or parallel connection)

Resistors: 220Ω for LED, 10KΩ for button pull-down

Power Source: 5V from Arduino


Circuit Design

The LED and buzzer activate simultaneously to signal the player.

The button input is read by the Arduino to calculate the reaction time.

The LCD displays the reaction time in milliseconds.


Key Features

Measures reaction time with millisecond accuracy
Includes both visual and auditory cues
Uses real-time input processing
Demonstrates hardware-software integration

Team Information

Project Title: Reaction Time Tester

Group: 17

Department: Computer Engineering
Members:
NANA AKOSUA ADDIPA – 1813522
TEMILOLUWA IFEDAPO TITILOYE – 1828822
GABRIEL NII ATTOH QUAYE – 1827522
ASARE KINGSLEY DONKOR – 1818122
BISMARK AMOAH – 1816222
RANDY AGYEKUM – 1814622
DADZIE KWESI ODARTEY - 1820722
SABLAH FREDRICK – 7108821
OWUSU TAKYI DANIEL – 7107621
MARFO RICHMOND MENSAH - 7102721

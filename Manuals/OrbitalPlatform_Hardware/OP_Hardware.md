# Orbital Platform
The Orbital Platform is the primary computer for which IntelliSat was created. It features an STM32 processor along with external memory and avionics. Details on the chips can be found on the [SSS Electrical Team's Github repo](https://github.com/uwu64/orbital-platform)

Since IntelliSat allows access to certain Orbital Platform resources, it uses specific names for those resources. These names that might not align with the labels on the board, especially as IntelliSat tries to stay compatible with the different revisions of Orbital Platform. \
This document shows different sections of the Orbital Platform board, and what names IntelliSat gives them.

# OP Revision 1

<img src="../../img/physcial_settings/OP_R1.jpg" width=500 height=500>

| ID | Name          | User Interaction                      |
|----|---------------|---------------------------------------|
| 01 | SWD Connector | Used to program and debug the board   |
| 02 | LED A         | Can be toggled via _led.h_            |
| 03 | LED B         | Can be toggled via _led.h_            |
| 04 | LED C         | Can be toggled via _led.h_            |
| 05 | LED Heartbeat | Blinks every 1 ms                     |
| 06 | LED Fault     | Can be toggled via _led.h_            |
| 07 | LED All Good  | Can be toggled via _led.h_            |
| 09 | LED Dx        | Can be toggled via _led.h_            |
| 10 | Button 0      | Status can be accessed via _button.h_ |
| 11 | Button 1      | Status can be accessed via _button.h_ |

Additionally, there is a large sections of pins sticking out on the right side. __The labels show which peripherals use the pins__.  \
Details on which of these peripherals and pins are used under IntelliSat can be found in the Hardware Configurations google sheet.

# OP Revision 2

<img src="../../img/physcial_settings/OP_R2.jpg" width=500 height=500>

| ID | Name          | User Interaction                      |
|----|---------------|---------------------------------------|
| 01 | SWD Connector | Used to program and debug the board   |
| 02 | LED Heartbeat | Blinks every 1 ms                     |
| 03 | LED Dx        | Can be toggled via _led.h_            |
| 04 | Button 0      | Status can be accessed via _button.h_ |
| 05 | Button 1      | Status can be accessed via _button.h_ |

Additionally, there is a small set of pins sticking out on the right side. Details on these pins: ... . \
Details on which of these peripherals and pins are used under IntelliSat can be found in the Hardware Configurations google sheet.
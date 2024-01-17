## Pre-requisites
This section assumed the following:
- You have the Orbital Platform with you
- You have an ST-LINK
- You have jumper cables, in case the ST-LINK has none already

# Connecting the board
The ST-LINK USB Connector already has wires attached to it, with labels on each wire.

## Orbital Platform Revision 1 & 2
The [Orbital Platform Hardware details](./OP_Hardware.md) document shows the SWD connector pins. This is where the ST-LINK connector is supposed to be connected to. Since the labels for these pins is different from that on the ST-LINK (and the ST-LINK has more pins) the following table shows how to connect the pins:

| ST-LINK | SWD Connector |
|---------|---------------|
|   3     | 3.3V          |
|   G     | GND           |
|   C     | SWCLK         |
|   D     | SWDIO         |
|   R     | RST           |


Once connected, the PC should power the Orbital Platform. This will cause the Orbital Platform to start running whatever program was last loaded on this. In case the program is IntelliSat, atleast one of the LEDs should be blinking at a constant rate.
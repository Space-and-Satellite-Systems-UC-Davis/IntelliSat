# IntelliSat
Welcome to code respository for the Space And Systems Satellite Club's REALOP 1 Flight Software! The code here will be run on our flight computer, the Orbital Platform, which will be responsible for controlling and monitoring the CubeSat's various subsystems. Details regarding the Orbital Platform are available on this github repository: [uwu64/orbital-platform](https://github.com/uwu64/orbital-platform).

## Getting Started
To get started with our codebase, you'll need to have a basic understanding of C programming. 
In order to work on the codebase itself, you'll need the tools to be able to clone, build, and eventually flash and debug the code. For this, refer to the following section:
- [Getting Started](./Manuals/Getting_Started.md)

For those developing code, many elements might be hardware specific. Reference manuals, datasheets, and programming guides of the various chips on the Orbital Platform are located under:
- [Reference_Manuals](./Manuals/Reference_Manuals/)

# Note about Board Revisions
The Orbital Platform goes through revisions. IntelliSat attempts to stay functional on each revision. However, certain GPIO and hardware configurations change across revisions, and thus change within IntelliSat. When using IntelliSat, keep the following in mind:
- Based on the Revision you are working with, make sure to change the `OP_REV` macro in `Src/globals.h` before building or flashing IntelliSat
- Differences between the Revisions can be found [here](./Manuals/OrbitalPlatform_Hardware/OP_Hardware.md)
    - Certain LED function names differ between revisions, so it's important to know those
# Working with the Orbital Platform

## Physically connecting to the board
We use ST-LINK V2 to connect to the board. The ST-LINK USB Connector already has wires attached to it, with labels on each wire.

IMAGE ../../img/physical_settings/

The Orbital Platform has a connector port on it's topside, with labels that correspond to the wires on the ST-LINK Connector. 

IMAGE ../../img/physical_settings/

Connect the Port and the Connector to each other by matching the labels.
If the compuer is powered on, the Platform should start getting power, and start running whatever program is loaded on it. Since the `Heartbeat` LED blinks every second in our FSW, it's blinking will be a clear indication of things working.

IMAGE ../../img/physical_settings/


## Flashing the Platform
With ST-LINK V2, fashing can be done right through the CUBE IDE. Before flashing, make sure the Orbital Platform is connected. Then make sure the REALOP Project is open in the IDE. Flashing can be done by pressing the `Run` in the top pane of the IDE.

![RunButton](../../img/work/cube_ide_run_button.png)

This will first rebuild the project and then try connecting to the Platform. Once both are done, it will flash the program onto the Platform

**NOTE**: Once this is completed, the Platform will start running the code immediately.

## Debugging the Platform
In order to debug, make sure to have a breakpoint at some line in your code (ideally close to whichever component you think is failing). Then, make sure the Platform is physically connected. Press the `Debug` icon on the top pane. There will be a prompt to enter the `Debugger Layout`. Press `Yes`.

![DebugButton](../../img/work/cube_ide_debug_button.png)

The debugger will open up

IMAGE ../../img/work/

The project will be built and flashed onto the Platform, and the IDE will have the normal tools found on most debuggers (such as single stepping, stepping over, watching expressions, looking at registers, etc), and will stop execution once it hits a breakpoint).
**NOTE**: The Startup file might open whenver the program starts. This is normal behavior.
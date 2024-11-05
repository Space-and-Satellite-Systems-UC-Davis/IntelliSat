#ifndef _TASK_H_
#define _TASK_H_

#include "schedulerGlobals.h"
// #include "./../integration/integration.h"

#include "status.h"

typedef void (*FunctionPointer)();  // For mode functions

/*
 * Task - a mode of execution on satellite
 * (note: uses preemption to interrupt running tasks)
 */
struct Task {
    uint8_t task_id;            // PRIMARY_KEY
    uint32_t taskInterrupts;    // times taskISR called, cancel mode/task after x reached
    FunctionPointer configPtr;  // configure timers, other mode info.
    FunctionPointer runPtr;     // the main func. for mode, via ADCS
    FunctionPointer cleanPtr;   // reset timers, clear temp buffers, etc
    uint8_t func1;              // Open functionality
};

extern volatile struct Task currTask;
extern struct Task taskTable[6];

/* Scheduling methods */
bool lowPwrTime(); // tautology (charging is idle mode)
bool detumbleTime();
bool commsTime();
int experimentTime();
bool eccTime();

/* Configure methods */
void configLowPwr();
void configDetumble();
void configComms();
void configExperiment();
void configEcc();
void configIdle();

/* Run methods */
void lowPwr();
void detumble();
void comms();
void experiment();
void ecc();
void idle();

/* Clean methods */
void cleanLowPwr();
void cleanDetumble();
void cleanComms();
void cleanExperiment();
void cleanEcc();
void cleanIdle();

#endif

#include "hooks.h"
#include "print_scan.h"

void vApplicationMallocFailedHook(void)
{
    printMsg("MALLOC FAILED - Out of heap memory!\n");
    taskDISABLE_INTERRUPTS();
    for(;;);  // Halt for debugging
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printMsg("STACK OVERFLOW in task: %s\n", pcTaskName);
    taskDISABLE_INTERRUPTS();
    for(;;);  // Halt for debugging
}
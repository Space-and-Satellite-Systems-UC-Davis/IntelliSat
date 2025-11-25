/*
write to vector table the exception handler address
shcsr

progress:
test fault trigger.
    find fault triggered through debugger->SFRs->CFSR
    IMPRECISERR = bus fault


documentation
init handler, or else hard fault will occur by default
Fault handler headers defined in startup_stm32l476zgtx.s
CFSR indicates the cause of a MemManage fault, BusFault, or UsageFault
    defined in 4.3.10 of Cortex M4 User Guide

note:
memfault enabled in MPU enable and disabled in MPU diable
The processor uses Handler mode to handle all exceptions except for reset
    Handler mode always uses the MSP, 


TODO: test different faults
    test fault address
*/

#include "fault_handler.h"
#include "../LED/led.h"

#define LOOP_AT_COMPLETION 0

void printOutFault(void);
void printTaskStatus(void);

volatile unsigned int stacked_r0;
volatile unsigned int stacked_r1;
volatile unsigned int stacked_r2;
volatile unsigned int stacked_r3;
volatile unsigned int stacked_r12;
volatile unsigned int stacked_lr;
volatile unsigned int stacked_pc;
volatile unsigned int stacked_psr;
volatile unsigned int _CFSR;
volatile unsigned int _BFAR;
volatile unsigned int _MMAR;
// TaskStatus_t 	xTaskDetails;
// TaskStatus_t 	TaskStatus;

void fault_handler_init(){
    // Enable MemManage, BusFault, and UsageFault fault handlers
    // Using just Hard Fault Handler for all faults for now
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_USGFAULTENA_Msk;
}

__attribute__( ( naked ) ) void HardFault_Handler(void) {
    __asm volatile(
        "tst lr, #4              \n"  // Check EXC_RETURN in LR bit 2
        "ite eq                   \n"
        "mrseq r0, msp           \n"  // if 0, stack frame on MSP
        "mrsne r0, psp           \n"  // if 1, stack frame on PSP
        "mov r1, lr               \n"  // Pass EXC_RETURN as second argument
        "b HardFault_Handler_C    \n"  // Branch to C handler
    );
}

/* HardFault handler in C, with stack frame location and LR value extracted
   from the assembly wrapper as input parameters */
void HardFault_Handler_C(unsigned int *hardfault_args)
{
	_BFAR           = SCB->BFAR;
	_MMAR 			= SCB->MMFAR;
	_CFSR 			= SCB->CFSR;

  stacked_r0  = ((unsigned int) hardfault_args[0]);
  stacked_r1  = ((unsigned int) hardfault_args[1]);
  stacked_r2  = ((unsigned int) hardfault_args[2]);
  stacked_r3  = ((unsigned int) hardfault_args[3]);
  stacked_r12 = ((unsigned int) hardfault_args[4]);
  stacked_lr  = ((unsigned int) hardfault_args[5]);
  stacked_pc  = ((unsigned int) hardfault_args[6]);
  stacked_psr = ((unsigned int) hardfault_args[7]);

		printMsg("\n\r==== [HardFault_Handler] ====\n\r");
	  printOutFault();

// #if(USING_FREE_RTOS == 1)
// 	vTaskGetTaskInfo( xTaskGetCurrentTaskHandle(), &taskStatus, pdTRUE, eInvalid);
// 	printTaskStatus();
// #endif

#if(LOOP_AT_COMPLETION == 1)
	  while(1) __asm("BKPT #0\n") ; // Break into the debugger
#else
		NVIC_SystemReset();
#endif
}

// MemManage faults can be caused by violation of the access rules which are defined by the MPU configuration
void MemManage_Handler(void){
    volatile int32_t cfsr = SCB->CFSR;

    if (cfsr & SCB_CFSR_MMARVALID_Msk){
        printMsg("MemManage Fault: MMAR holds a valid fault address.");
        //TODO: If a MemManage fault occurs and is escalated to a HardFault because of priority, the HardFault handler must set this bit to 0. 

    }

    // Occurs during lazy floating-point state preservation 
    if (cfsr & SCB_CFSR_MLSPERR_Msk){
        printMsg("MemManage Fault: MLSPERR");

    }

    if (cfsr & SCB_CFSR_MSTKERR_Msk){
        printMsg("MemManage Fault: MSTKERR");
    }

    if (cfsr & SCB_CFSR_MUNSTKERR_Msk){
        printMsg("MemManage Fault: MUNSTKERR");
    }

    if (cfsr & SCB_CFSR_DACCVIOL_Msk){
        printMsg("MemManage Fault: DACCVIOL");
    }

    if (cfsr & SCB_CFSR_IACCVIOL_Msk){
        printMsg("MemManage Fault: IACCVIOL");
    
    }
    while(1);
}

void BusFault_Handler(void){
    int32_t cfsr = SCB->CFSR;

    if (cfsr & SCB_CFSR_BFARVALID_Msk){
        printMsg("Bus Fault: BFAR holds a valid fault address");

    }

    // a bus fault occurred during floating-point lazy state preservation
    if (cfsr & SCB_CFSR_LSPERR_Msk){
        printMsg("Bus Fault: LSPERR");

    }

    if (cfsr & SCB_CFSR_STKERR_Msk){
        printMsg("Bus Fault: STKERR");

    }

    if (cfsr & SCB_CFSR_UNSTKERR_Msk){
        printMsg("Bus Fault: UNSTKERR");

    }

    if (cfsr & SCB_CFSR_IMPRECISERR_Msk){
        printMsg("Bus Fault: IMPRECISERR");
    }

    if (cfsr & SCB_CFSR_PRECISERR_Msk){
        printMsg("Bus Fault: PRECISERR");

    }

    if (cfsr & SCB_CFSR_IBUSERR_Msk){
        printMsg("Bus Fault: IBUSERR");
        
    }
    while(1);
}

void UsageFault_Handler(void){
    int32_t cfsr = SCB->CFSR;
    
    if (cfsr & SCB_CFSR_DIVBYZERO_Msk){
        printMsg("Usage Fault: DIVBYZERO");

    }

    if (cfsr & SCB_CFSR_UNALIGNED_Msk){
        printMsg("Usage Fault: UNALIGNED");

    }

    if (cfsr & SCB_CFSR_NOCP_Msk){
        printMsg("Usage Fault: NOCP");

    }

    if (cfsr & SCB_CFSR_INVPC_Msk){
        printMsg("Usage Fault: INVPC");

    }

    if (cfsr & SCB_CFSR_INVSTATE_Msk){
        printMsg("Usage Fault: INVSTATE");

    }

    if (cfsr & SCB_CFSR_UNDEFINSTR_Msk){
        printMsg("Usage Fault: UNDEFINSTR");

    }
    while(1);
}

void printOutFault(void)
{
	// unixRTC_TimeNow = getUnixTimeSecondsFromRTC();
	// gtimeRTC_TimeNow = gmtime(&unixRTC_TimeNow);

	// printMsg("Hard Fault at Time: - UTC %04d-%02d-%02d  %02d:%02d:%02d\n\r",
	// 				gtimeRTC_TimeNow->tm_year,
	// 				gtimeRTC_TimeNow->tm_mon,
	// 				gtimeRTC_TimeNow->tm_mday,
	// 				gtimeRTC_TimeNow->tm_hour,
	// 				gtimeRTC_TimeNow->tm_min,
	// 				gtimeRTC_TimeNow->tm_sec
	// 			);
	printMsg("- Stack frame:\n\r");
	printMsg(" R0  = 0x%08x\n\r", (unsigned int)stacked_r0);
	printMsg(" R1  = 0x%08x\n\r", (unsigned int)stacked_r1);
	printMsg(" R2  = 0x%08x\n\r", (unsigned int)stacked_r2);
	printMsg(" R3  = 0x%08x\n\r", (unsigned int)stacked_r3);
	printMsg(" R12 = 0x%08x\n\r", (unsigned int)stacked_r12);
	printMsg(" LR  = 0x%08x\n\r", (unsigned int)stacked_lr);
	printMsg(" PC  = 0x%08x\n\r", (unsigned int)stacked_pc);
	printMsg(" PSR = 0x%08x\n\r", (unsigned int)stacked_psr);
	printMsg("- FSR/FAR:\n\r");
	printMsg(" CFSR = 0x%08x\n\r", (unsigned int)_CFSR);
	printMsg(" HFSR = 0x%08x\n\r", (unsigned int)SCB->HFSR);
	printMsg(" DFSR = 0x%08x\n\r", (unsigned int)SCB->DFSR);
	printMsg(" AFSR = 0x%08x\n\r", (unsigned int)SCB->AFSR);

	if (_CFSR & SCB_CFSR_MMARVALID_Msk) printMsg(" Memory Management Fault Addr = 0x%08x\n\r", (unsigned int)_MMAR);
	if (_CFSR & SCB_CFSR_BFARVALID_Msk) printMsg(" Bus Fault Addr     = 0x%08x\n\r", (unsigned int)_BFAR);

	if (_CFSR & SCB_CFSR_MLSPERR_Msk) printMsg(" Memory Management Fault - MLSPERR: Floating-point lazy state preservation \n\r");
	if (_CFSR & SCB_CFSR_MSTKERR_Msk) printMsg(" Memory Management Fault - MSTKERR: stacking for an exception entry has caused one or more access violations \n\r");
	if (_CFSR & SCB_CFSR_MUNSTKERR_Msk) printMsg(" Memory Management Fault - MUNSTKERR: unstack for an exception return has caused one or more access violations. \n\r");
	if (_CFSR & SCB_CFSR_DACCVIOL_Msk) printMsg(" Memory Management Fault - DACCVIOL: load or store at a location that does not permit the operation. \n\r");
	if (_CFSR & SCB_CFSR_IACCVIOL_Msk) printMsg(" Memory Management Fault - IACCVIOL: instruction fetch from a location that does not permit execution. \n\r");

	if (_CFSR & SCB_CFSR_LSPERR_Msk) printMsg(" Bus Fault - LSPERR: Floating-point lazy state preservation. \n\r");
	if (_CFSR & SCB_CFSR_STKERR_Msk) printMsg(" Bus Fault - STKERR: stacking for an exception entry has caused one or more BusFaults. \n\r");
	if (_CFSR & SCB_CFSR_UNSTKERR_Msk) printMsg(" Bus Fault - UNSTKERR: unstack for an exception return has caused one or more BusFaults. \n\r");
	if (_CFSR & SCB_CFSR_IMPRECISERR_Msk) printMsg(" Bus Fault - IMPRECISERR: a data bus error has occurred, the PC return address is not valid \n\r");
	if (_CFSR & SCB_CFSR_PRECISERR_Msk) printMsg(" Bus Fault - PRECISERR: a data bus error has occurred, the PC return address is valid \n\r");
	if (_CFSR & SCB_CFSR_IBUSERR_Msk) printMsg(" Bus Fault - IBUSERR: instruction bus error \n\r");


    if (_CFSR & SCB_CFSR_DIVBYZERO_Msk) printMsg(" Usage Fault - DIVBYZERO: the processor has executed an SDIV or UDIV instruction with a divisor of 0. \n\r");
    if (_CFSR & SCB_CFSR_UNALIGNED_Msk) printMsg(" Usage Fault - UNALIGNED: the processor has made an unaligned memory access. \n\r");
    if (_CFSR & SCB_CFSR_NOCP_Msk) printMsg(" Usage Fault - NOCP: the processor has attempted to access a coprocessor. \n\r");
    if (_CFSR & SCB_CFSR_INVPC_Msk) printMsg(" Usage Fault - INVPC: illegal load of EXC_RETURN to the PC, due to invalid context, or an invalid EXC_RETURN value. \n\r");
	if (_CFSR & SCB_CFSR_INVSTATE_Msk) printMsg(" Usage Fault - INVSTATE: attempted to execute an instruction that makes illegal use of the EPSR. \n\r");
    if (_CFSR & SCB_CFSR_UNDEFINSTR_Msk) printMsg(" Usage Fault - UNDEFINSTR: attempted to execute an undefined instruction. \n\r");
    
    printMsg("======= [End] =======\r\r\n");
	printMsg("\n");
}
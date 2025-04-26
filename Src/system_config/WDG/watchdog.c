#include "watchdog.h"
#include <SunSensors/sun_sensors.h>

static int WWDG_timeout;
static bool config_iwdg = false;
static bool config_wwdg = false;


void watchdog_iwdg_config(int ms){
    //if configured once, no need to do it again
    if(config_iwdg){
        return;
    }
    config_iwdg = true;

    //configure IWDG registers
    IWDG->KR = IWDG_START;
    IWDG->KR = IWDG_ENABLE;
    IWDG->PR = IWDG_PR_PR_1; 

    //calculate reload value
    int rlr = ms * 2 - 1; 

    IWDG->RLR = rlr; //actually do math here later
    while(FLASH->SR & FLASH_SR_BSY_Msk);
    IWDG->KR = IWDG_KICK;
}

void watchdog_wwdg_config(int ms){
    //if configured once, no need to do it again
    if(config_wwdg){
        return;
    }
    config_wwdg = true;

    //configure WWDG
    RCC->APB1ENR1 |= RCC_APB1ENR1_WWDGEN;
    RCC->APB1SMENR1 |= RCC_APB1SMENR1_WWDGSMEN;
    WWDG->CFR |= WWDG_CFR_WDGTB;
    float seconds = ms / 1000.0;
    WWDG_timeout = seconds / WWDG_CALCULATE_TIMEOUT - 1;
    WWDG->CR |= WWDG_CR_WDGA | WWDG_CR_T_6 | WWDG_timeout;
}

//timeout is approx 0.5 seconds for IWDG,
//timeout is 262144 cycles ~50ms
void watchdog_hardware_config() {
	IWDG->KR = IWDG_KICK; //PLEASE DO NOT REMOVE :) (Just deal with the consequences i guess if you really want to)
	WWDG->CR |= WWDG_CR_T_6 | WWDG_timeout;

    if(!(FLASH->OPTR & (FLASH_OPTR_IWDG_SW |FLASH_OPTR_WWDG_SW))){
        return;
    }
    //if reset by watchdog or flash loading, don't need to reconfigure.
    printMsg("RCC IS: %x" ,RCC->CSR);
    /*if(RCC->CSR & RCC_CSR_OBLRSTF || RCC->CSR & RCC_CSR_IWDGRSTF || RCC->CSR & RCC_CSR_WWDGRSTF){
        if(RCC->CSR & RCC_CSR_OBLRSTF){
            printMsg("Reset by option bytes! \n");
        } else if(RCC->CSR & RCC_CSR_IWDGRSTF){
            printMsg("Reset by IWDG!\n");
        }else{
            printMsg("Reset by WWDG!\n");
        }
    	//RCC->CSR |= RCC_CSR_RMVF;
        return;
    }*/
    //enable hardware watchdog

    while(FLASH->SR & FLASH_SR_BSY_Msk);
    //unlock FLASH_KEYR
    FLASH->KEYR = KEY1;
    FLASH->KEYR = KEY2;

    //unlock FLASH_OPTKEYR
    FLASH->OPTKEYR = OPTKEY1;
    FLASH->OPTKEYR = OPTKEY2;

    //check bsy bit in FLASH_SR
    while(FLASH->SR & FLASH_SR_BSY_Msk);

    //enable hardware watchdog for IWDG and WWDG
    FLASH->OPTR &= ~FLASH_OPTR_IWDG_SW_Msk;
    FLASH->OPTR &= ~FLASH_OPTR_WWDG_SW_Msk;

    //set option start bit in Flash control
    FLASH->CR |= FLASH_CR_OPTSTRT_Msk;

    //wait for bsy bit to be cleared
    while(FLASH->SR & FLASH_SR_BSY_Msk);

    //option byte loading
    FLASH->CR |= FLASH_CR_OBL_LAUNCH_Msk;

}

void watchdog_kick(){
	int count = 0;
	while(1){
		IWDG->KR |= IWDG_KICK;
        WWDG->CR |= WWDG_CR_T_6 | WWDG_timeout;
		//printMsg("%d\n", count++);
        count++;
        if(count > 1000000){
            printMsg("uh oh\n");
            delay_ms(400);
            count = 0;
            //int x = sun_sensors_readVoltage(PANEL0,DIODE0);
            //printMsg("voltage: %f\n", x);
            //while(1);
        }
	}
}

void whoReset(){
    if(RCC->CSR & RCC_CSR_WWDGRSTF){
        printMsg("Reset by WWDG!\n");
    } else if(RCC->CSR & RCC_CSR_IWDGRSTF){
        printMsg("Reset by IWDG!\n");
    }
    RCC->CSR |= RCC_CSR_RMVF;

}
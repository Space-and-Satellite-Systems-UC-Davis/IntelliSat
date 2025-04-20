#include "watchdog.h"
#include <SunSensors/sun_sensors.h>

//timeout is approx 0.5 seconds for IWDG,
//timeout is 262144 cycles ~50ms
void watchdog_config() {
	IWDG->KR = IWDG_KICK; //PLEASE DO NOT REMOVE :) (Just deal with the consequences i guess if you really want to)
	//WWDG->CR |= WWDG_KICK;

    //if reset by watchdog or flash loading, don't need to reconfigure.
    if(RCC->CSR & RCC_CSR_OBLRSTF || RCC->CSR & RCC_CSR_IWDGRSTF || RCC->CSR & RCC_CSR_WWDGRSTF){
    	RCC->CSR |= RCC_CSR_RMVF;
        if(RCC->CSR & RCC_CSR_OBLRSTF){
            printMsg("Reset by option bytes! \n");
        } else if(RCC->CSR & RCC_CSR_IWDGRSTF){
            printMsg("Reset by IWDG!\n");
        }else{
            printMsg("Reset by WWDG!\n");
        }
        return;
    }
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
    //FLASH->OPTR &= ~FLASH_OPTR_WWDG_SW_Msk;

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
		IWDG->KR = IWDG_KICK;
        //WWDG->CR |= WWDG_KICK;
		//printMsg("%d\n", count++);
        count++;
        if(count > 1000){
            printMsg("uh oh%d\n", count);
            int x = sun_sensors_readVoltage(PANEL0,DIODE0);
            printMsg("voltage: %f\n", x);
            //while(1);
        }
	}
}


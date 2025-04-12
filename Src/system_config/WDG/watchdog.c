#include "watchdog.h"


//timeout is approx 0.5 seconds for IWDG,
//timeout is 262144 cycles ~50ms
void watchdog_config() {

    //if reset by watchdog or flash loading, don't need to reconfigure.
    if(RCC->CSR & RCC_CSR_OBLRSTF || RCC->CSR & RCC_CSR_IWDGRSTF){
        printMsg("return");
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

    //clear OPTKEYR again??
    //FLASH->OPTKEYR = OPTKEY1;
    //			FLASH->OPTKEYR = OPTKEY2;

    //enable hardware watchdog for IWDG and WWDG
    FLASH->OPTR &= ~FLASH_OPTR_IWDG_SW_Msk;
    FLASH->OPTR &= ~FLASH_OPTR_WWDG_SW_Msk;

    //set option start bit in Flash control
    FLASH->CR |= FLASH_CR_OPTSTRT_Msk;

    //wait for bsy bit to be cleared
    while(FLASH->SR & FLASH_SR_BSY_Msk);

    //option byte loading
    FLASH->CR |= FLASH_CR_OBL_LAUNCH_Msk;

    printMsg("unlocked!");

}

void watchdog_kick(){
	int count = 0;
	while(1){
		IWDG->KR = 0x0000AAAA;
		printMsg("%d\n", count++);
	}
}


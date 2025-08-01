#include "watchdog.h"
#include <SunSensors/sun_sensors.h>

static int WWDG_timeout;

static int lastTime = 0;


void watchdog_config(int ms){
    watchdog_iwdg_config(ms);
    watchdog_wwdg_config(ms);
    watchdog_interrupt_config(ms);
}

void watchdog_iwdg_config(int ms){
    //configure IWDG registers
    IWDG->KR = IWDG_START;
    IWDG->KR = IWDG_ENABLE;
    IWDG->PR = IWDG_PR_PR_1; 

    //calculate reload value
    int rlr = ms * 2 - 1; 

    IWDG->RLR = rlr; 
    while(FLASH->SR & FLASH_SR_BSY_Msk);
    IWDG->KR = IWDG_KICK;
}

void watchdog_wwdg_config(int ms){
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

void watchdog_interrupt_config(int ms){

    ms -= 10; //give some time before the actual timeout time just in case

    uint16_t PSC = 999; //To make calculating ARR easier because divide by 1000
    uint16_t ARR = APB1_FREQ * ms * 2 - 1 ; //ARR = (5MgHz(ms)/(PSC+1))-1 = 5*ms*10^3/1000 - 1 = 5*ms - 1
    
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN; //Enable Tim 3
    WATCHDOG_TIMER->CR1 &= ~TIM_CR1_CEN; //Disable timer from control register
    WATCHDOG_TIMER->CR1 |= TIM_CR1_URS; //Generate UG event on rollovers
    WATCHDOG_TIMER->CR1 |= TIM_CR1_DIR; //downcounting

    WATCHDOG_TIMER->PSC = PSC; //set psc
	WATCHDOG_TIMER->ARR = ARR; //set arr

    WATCHDOG_TIMER->DIER |= TIM_DIER_UIE; //enable timer interrupt
    NVIC_EnableIRQ(TIM3_IRQn); //enable tim3 interrupt

    //enable timer
    WATCHDOG_TIMER->CR1 |= TIM_CR1_CEN;

}

void TIM3_IRQHandler(){
    if(WATCHDOG_TIMER->SR & TIM_SR_UIF) {
		WATCHDOG_TIMER->SR &= ~TIM_SR_UIF; //check for interrupt flag and update
    }
    IWDG->KR |= IWDG_KICK; //kick iwdg
    WWDG->CR |= WWDG_CR_T_6 | WWDG_timeout; //kick wwdg

    //test timeout time
    /*int time = getSysTime();
    int diff = time-lastTime;
    lastTime = time;
    printMsg("%d, kicked!\n", diff);*/
}



void watchdog_kick(){
	int count = 0;
	while(1){
		IWDG->KR |= IWDG_KICK;
        WWDG->CR |= WWDG_CR_T_6 | WWDG_timeout;
		//printMsg("%d\n", count++);
        //count++;
        //if(count > 1000000){
            //printMsg("uh oh\n");
            //delay_ms(400);
            //count = 0;
            //int x = sun_sensors_readVoltage(PANEL0,DIODE0);
            //printMsg("voltage: %f\n", x);
            //while(1);
        //}
	}
}
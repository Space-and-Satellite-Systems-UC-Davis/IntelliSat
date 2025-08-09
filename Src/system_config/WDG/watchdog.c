#include "watchdog.h"
#include <SunSensors/sun_sensors.h>
#include <print_scan.h>

static int WWDG_timeout;
static int IWDG_timeout;

void watchdog_init(int ms){
    IWDG_timeout = ms;
    watchdog_iwdg_config(ms);
    if(ms > WWDG_MAX){
        ms = WWDG_MAX;
    }
    watchdog_wwdg_config(ms);
    watchdog_interrupt_config(ms);
}

void watchdog_iwdg_config(int ms){
    //configure IWDG registers
    IWDG->KR = IWDG_START;
    IWDG->KR = IWDG_ENABLE;

    float pr = ms / T_LSI / (IWDG_RLR_RL + 1);

    int count = 0;
    while(pr > 1){
        count ++;
        pr /= 2;
    }

    IWDG->PR = count; 
    

    //calculate reload value
    int rlr = ms / T_LSI / (1<<count) - 1;
    IWDG->RLR = rlr;

    while(FLASH->SR & FLASH_SR_BSY_Msk);
    IWDG->KR = IWDG_KICK;
}

void watchdog_wwdg_config(int ms){
    //configure WWDG
    RCC->APB1ENR1 |= RCC_APB1ENR1_WWDGEN;
    RCC->APB1SMENR1 &= ~RCC_APB1SMENR1_WWDGSMEN; //disable during sleep
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
    // printMsg("KICKED");
    //test timeout time
    /*int time = getSysTime();
    int diff = time-lastTime;
    lastTime = time;
    printMsg("%d, kicked!\n", diff);*/
}

void watchdog_IWDGSleepMode()
{
    watchdog_changeIWDGTimeout(IWDG_MAX_TIMEOUT);
}

void watchdog_IWDGWakeUp()
{
    watchdog_changeIWDGTimeout(IWDG_timeout);
}

void watchdog_changeIWDGTimeout(int ms){
    if(ms > IWDG_timeout){//if interval gets longer, configure iwdg before interrupt
        watchdog_iwdg_config(ms);
        watchdog_interrupt_config(ms);
    }else{ //if interval gets shorter, configure interrupt before iwdg
        watchdog_interrupt_config(ms);
        watchdog_iwdg_config(ms);
    }
    
}
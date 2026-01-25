#include "dbg_config.h"
#include "stm32l476xx.h"

void debug_init() {
    // Clear the configs by default to avoid carry over to next flash cycle.
    DBGMCU->APB1FZR1 &= DBGMCU_APB1FZR1_DBG_WWDG_STOP_Msk;
    DBGMCU->APB1FZR1 &= DBGMCU_APB1FZR1_DBG_IWDG_STOP_Msk;
    // Pause the WWDG counter when core also pause in debug mode
    DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_WWDG_STOP_Msk;
    // Pause the IWDG counter when core also pause in debug mode
    DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_IWDG_STOP_Msk;
}
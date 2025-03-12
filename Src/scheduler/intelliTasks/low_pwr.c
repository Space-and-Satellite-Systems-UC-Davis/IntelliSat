#include "low_pwr.h"
#include "task.h"
// #include "eta3000.h"     // Dedicated balancing IC
// #include "s8254a.h"      // Protection IC

static bool low_pwr_mode = false;
static TaskHandle_t low_pwr_task = NULL;

// IC Interface ============================================================
static uint8_t read_battery_percent(void) {
    // Read from dedicated battery ICs
    // return s8254a_read_battery_level();  // Example for S-8254A
}

static bool battery_is_low(void) {
    return read_battery_percent() <= LOW_BATTERY_THRESHOLD;
}

static bool battery_is_recovered(void) {
    return read_battery_percent() >= (LOW_BATTERY_THRESHOLD + BATT_HYSTERESIS);
}

// Mode Management =========================================================
bool low_pwr_time(void) {
    static bool was_low = false;
    const bool is_low = battery_is_low();
    const bool recovered = battery_is_recovered();

    if(!was_low && is_low) {
        low_pwr_mode = true;
        was_low = true;
    }
    else if(was_low && recovered) {
        low_pwr_mode = false;
        was_low = false;
    }
    
    return low_pwr_mode;
}

void config_low_pwr(void) {
    // Suspend lower priority tasks
    for(int i = 0; i < TASK_TABLE_LEN; i++) {
        intelli_task_t *task = &task_table[i];
        if(task->func_1 < LOW_PWR_PRIORITY_THRESHOLD) {
            vTaskSuspend(task->FreeRTOS_handle);
        }
    }
    
    // Enable hardware low-power features
    enable_low_power_hardware();
    
    // Let dedicated ICs manage battery
    // eta3000_enable_autobalance(true);
    // s8254a_enable_protection(true);
}

void low_pwr(void) {
    // Only needs to maintain state, ICs handle actual management
    uint32_t notify_bits;
    
    // Handle external mode change requests
    if(xTaskNotifyWait(0, ULONG_MAX, &notify_bits, 0)) {
        if(notify_bits & LOW_PWR_NOTIFY_ENTER) {
            low_pwr_mode = true;
            config_low_pwr();
        }
        if(notify_bits & LOW_PWR_NOTIFY_EXIT) {
            low_pwr_mode = false;
            clean_low_pwr();
        }
    }
}

void clean_low_pwr(void) {
    // Resume suspended tasks
    for(int i = 0; i < TASK_TABLE_LEN; i++) {
        intelli_task_t *task = &task_table[i];
        if(task->func_1 < LOW_PWR_PRIORITY_THRESHOLD) {
            vTaskResume(task->FreeRTOS_handle);
        }
    }
    
    // Restore normal hardware operation
    restore_normal_hardware();
    // eta3000_enable_autobalance(false);
    // s8254a_enable_protection(false);
}

// Hardware Control ========================================================
void enable_low_power_hardware(void) {
    // Implementation-specific low-power modes:
    // - Reduce clock speeds
    // - Disable non-essential peripherals
    // - Adjust voltage regulators
    // __HAL_RCC_GPIOB_CLK_DISABLE();
    // HAL_PWREx_EnableUltraLowPower();
}

void restore_normal_hardware(void) {
    // Restore full operational state
    // __HAL_RCC_GPIOB_CLK_ENABLE();
    // HAL_PWREx_DisableUltraLowPower();
}
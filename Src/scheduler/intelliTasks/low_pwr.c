#include "low_pwr.h"
// TODO implement charge.h
#include "charge.h"

/// @brief Check if the bms should enter or eit low power mode.
/// @return The state of was_triggered.
bool low_pwr_time()
{
    // TODO implement get_battery_percent()
    uint8_t batt = 0;
    // uint8_t batt = get_battery_percent();

    if (!low_pwr_mode && batt < LOW_BATTERY_THRESHOLD)
    {
        low_pwr_mode = true;
    }
    else if (low_pwr_mode && batt >= CHARGE_COMPLETE_THRESHOLD)
    {
        low_pwr_mode = false;
    }
    return low_pwr_mode;
}

void config_low_pwr()
{
    for (int i = 0; i < TASK_TABLE_LEN; i++)
    {
        intelli_task_t *task = &task_table[i];
        if (task->func_1 < LOW_PWR_PRIORITY_THRESHOLD)
        { // Lower-priority tasks
            vTaskSuspend(task->FreeRTOS_handle);
        }
    }
    // TODO enable_low_power_hardware();
    // That is if it is desirable to do certain other thigns in low power mode
}
void low_pwr()
{
    manage_charging();
    // TODO Use FreeRTOS notifications for async events liek clean up
}

void clean_low_pwr()
{
    for (int i = 0; i < TASK_TABLE_LEN; i++)
    {
        intelli_task_t *task = &task_table[i];
        if (task->func_1 < LOW_PWR_PRIORITY_THRESHOLD)
        {
            vTaskResume(task->FreeRTOS_handle);
        }
    }
    // TODO enable_low_power_hardware();
    // That is if it is desirable to do certain other thigns in low power mode

}

uint8_t get_battery_percent()
{
    // TODO replace with actual ADC cause htf does that shit work
    return adc_singleConversion() * ADC_TO_PERCENT_SCALE;
}

// ? Should this be here? Also which pin is it?
void adc_conf()
{
    // Initializes ADC clocks
    void adc_init();

    // Enables the ADC
    void adc_enable();

    // Configures the GPIO pin C0 for the ADC
    void adc_configGPIO();

    // Configures the channel to set it to pin C0, and sets sampling time to 640.5 cycles per sample
    void adc_setChannel();
}

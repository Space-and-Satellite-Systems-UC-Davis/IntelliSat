#pragma once

/*
 * Host replacement for Src/system_config/LED/led.h: the provisioning entry only
 * initializes the LED pins because the project's TIM7 heartbeat calls blinky().
 */
void led_init(void);

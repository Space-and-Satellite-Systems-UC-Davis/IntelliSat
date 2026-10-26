#pragma once
#include "stm32l476xx.h"
#define GPIO_MODER_BITS_SPAN 2u
#define GPIO_MODER_Output 1u
#define GPIO_MODER_Analog 3u
void gpio_low(GPIO_TypeDef *port, int pin);
void gpio_high(GPIO_TypeDef *port, int pin);

#ifndef DMA_H
#define DMA_H

#include "stm32l476xx.h"
#include <globals.h>

typedef enum {
	DMA_ADC1 = (int) DMA1_Channel1,
	DMA_ADC2 = (int) DMA1_Channel2,
	DMA_ADC3 = (int) DMA1_Channel3

} DMAPeripherals;

typedef struct {
	DMA_Channel_TypeDef* channel;
	uint32_t* peripheral_addr;
	uint32_t* memory_addr;
	uint16_t length;
	bool circular;
} DMAConfig;

void configure_channel(
		DMAConfig config
);

#endif 

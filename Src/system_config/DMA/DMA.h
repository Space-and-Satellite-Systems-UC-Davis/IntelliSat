#ifndef DMA_H
#define DMA_H

#include "stm32l476xx.h"
#include <globals.h>

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

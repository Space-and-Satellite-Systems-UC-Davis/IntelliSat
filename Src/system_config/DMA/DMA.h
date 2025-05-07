#ifndef DMA_H
#define DMA_H

#include "stm32l476xx.h"
#include <globals.h>

void configure_channel(
		DMA_Channel_TypeDef* channel,
		uint32_t* peripheral_addr,
		uint32_t* memory_addr,
		size_t length
);

#endif 

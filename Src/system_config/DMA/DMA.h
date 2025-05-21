#ifndef DMA_H
#define DMA_H

#include "stm32l476xx.h"
#include <globals.h>

typedef struct DMAPeripheral {
	DMA_Channel_TypeDef* channel;
	uint8_t channel_select;
} DMAPeripheral;

typedef struct {
	DMAPeripheral* peripheral;
	uint32_t peripheral_addr;
	uint32_t memory_addr;
	uint16_t length;
	bool circular;
	bool on_by_default;
} DMAConfig;

void configure_channel(
		DMAConfig config
);

void dma_enable_channel(DMA_Channel_TypeDef* channel);
void dma_disable_channel(DMA_Channel_TypeDef* channel);

void DMA_initializePeripheralConstants();

#endif 

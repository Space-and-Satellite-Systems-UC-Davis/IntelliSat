#ifndef DMA_H
#define DMA_H

#include "stm32l476xx.h"
#include <globals.h>

typedef enum {
	DMA_ADC1 = (uint32_t) DMA1_Channel1,
	DMA_ADC2 = (uint32_t) DMA1_Channel2,
	DMA_ADC3 = (uint32_t) DMA1_Channel3,
	DMA_SPI1_RX = (uint32_t) DMA1_Channel2,
	DMA_SPI1_TX = (uint32_t) DMA1_Channel3,
	DMA_SPI2_RX = (uint32_t) DMA1_Channel4,
	DMA_SPI2_TX = (uint32_t) DMA1_Channel5,
	DMA_SPI3_RX = (uint32_t) DMA2_Channel1,
	DMA_SPI3_TX = (uint32_t) DMA2_Channel2

} DMAPeripherals;

typedef struct {
	DMA_Channel_TypeDef* channel;
	uint32_t* peripheral_addr;
	uint32_t* memory_addr;
	uint16_t length;
	bool circular;
	bool on_by_default;
} DMAConfig;

void configure_channel(
		DMAConfig config
);

void dma_enable_channel(DMA_Channel_TypeDef* channel);
void dma_disable_channel(DMA_Channel_TypeDef* channel);
#endif 

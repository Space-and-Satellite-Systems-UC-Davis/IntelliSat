#ifndef DMA_H
#define DMA_H

#include "stm32l476xx.h"
#include <globals.h>

typedef enum enum_DMAPeripherals {
	SELECT_ADC1,
	SELECT_ADC2,
	SELECT_ADC3,
	SELECT_SPI1_RX,
	SELECT_SPI1_TX,
	SELECT_SPI2_RX,
	SELECT_SPI2_TX,
	SELECT_SPI3_RX,
	SELECT_SPI3_TX,
} enum_DMAPeripherals;

typedef struct DMAPeripheral {
	DMA_Channel_TypeDef* channel;
	DMA_Request_TypeDef* channel_select;
	uint32_t channel_select_pos;
	uint8_t channel_select_value;
} DMAPeripheral;

typedef struct {
	enum_DMAPeripherals selection;
	uint32_t peripheral_addr;
	uint32_t memory_addr;
	uint16_t length;
	bool circular;
	bool on_by_default;
} DMAConfig;

void configure_channel(
		DMAConfig config
);

void dma_enable_channel(enum_DMAPeripherals selection);
void dma_disable_channel(enum_DMAPeripherals selection);

void DMA_initializePeripheralConstants();
DMAPeripheral* DMA_selectPeripheral(enum_DMAPeripherals);

#endif 

#ifndef DMA_H
#define DMA_H

#include "stm32l476xx.h"
#include <globals.h>

//No options for some uart because they aren't implemented
typedef enum enum_DMAPeripherals {
	SELECT_ADC1,
	SELECT_ADC2,
	SELECT_ADC3,
//	SELECT_SPI1_RX,
//	SELECT_SPI1_TX,
//	SELECT_SPI2_RX,
//	SELECT_SPI2_TX,
//	SELECT_SPI3_RX,
//	SELECT_SPI3_TX,
//	SELECT_USART1_RX,
//	SELECT_USART1_TX,
//	SELECT_USART2_RX,
//	SELECT_USART2_TX,
//	SELECT_LPUART1_RX,
//	SELECT_LPUART1_TX,
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
	uint8_t pdata_size;
	uint8_t mdata_size;
	bool circular;
	bool peripheral_to_memory;
	bool peripheral_increment;
	bool memory_increment;
	bool transfer_interrupt;
} DMAConfig;

void configure_channel(DMAConfig config);

void dma_enable_channel(enum_DMAPeripherals selection);
void dma_disable_channel(enum_DMAPeripherals selection);

void DMA_initializePeripheralConstants();
DMAPeripheral* DMA_selectPeripheral(enum_DMAPeripherals);

#endif 

/*
 * peripheral_defs.c
 *
 *  Created on: May 20, 2025
 */

#include "DMA/DMA.h"

DMAPeripheral dma_adc1, dma_adc2, dma_adc3,
	dma_spi1_rx, dma_spi1_tx,
	dma_spi2_rx, dma_spi2_tx,
	dma_spi3_rx, dma_spi3_tx,
	dma_usart1_rx, dma_usart1_tx,
	dma_usart2_rx, dma_usart2_tx,
	dma_lpuart1_rx, dma_lpuart1_tx;

DMAPeripheral* struct_DMA_ADC1 = &dma_adc1;
DMAPeripheral* struct_DMA_ADC2 = &dma_adc2;
DMAPeripheral* struct_DMA_ADC3 = &dma_adc3;

DMAPeripheral* struct_DMA_SPI1_RX = &dma_spi1_rx;
DMAPeripheral* struct_DMA_SPI1_TX = &dma_spi1_tx;

DMAPeripheral* struct_DMA_SPI2_RX = &dma_spi2_rx;
DMAPeripheral* struct_DMA_SPI2_TX = &dma_spi2_tx;

DMAPeripheral* struct_DMA_SPI3_RX = &dma_spi3_rx;
DMAPeripheral* struct_DMA_SPI3_TX = &dma_spi3_tx;

DMAPeripheral* struct_DMA_USART1_RX = &dma_usart1_rx;
DMAPeripheral* struct_DMA_USART1_TX = &dma_usart1_tx;

DMAPeripheral* struct_DMA_USART2_RX = &dma_usart2_rx;
DMAPeripheral* struct_DMA_USART2_TX = &dma_usart2_tx;

DMAPeripheral* struct_DMA_LPUART1_RX = &dma_lpuart1_rx;
DMAPeripheral* struct_DMA_LPUART1_TX = &dma_lpuart1_tx;

//Arguably it's better to expand the CSELR information when needed
//I argue it's easier to understand and debug (always see the info)
void DMA_initWrapper(DMAPeripheral* peripheral, DMA_Channel_TypeDef* channel, uint8_t cs) {
	peripheral->channel = channel;
	peripheral->channel_select_value = cs;

	if ((uint32_t)channel < (uint32_t)DMA1_CSELR_BASE) { //DMA1
		peripheral->channel_select = DMA1_CSELR;
	} else { //DMA2
		peripheral->channel_select = DMA2_CSELR;
	}

	switch ((uint32_t) channel) {
		case DMA1_Channel1_BASE:
		case DMA2_Channel1_BASE:
			peripheral->channel_select_pos = DMA_CSELR_C1S_Pos;
			break;

		case DMA1_Channel2_BASE:
		case DMA2_Channel2_BASE:
			peripheral->channel_select_pos = DMA_CSELR_C2S_Pos;
			break;

		case DMA1_Channel3_BASE:
		case DMA2_Channel3_BASE:
			peripheral->channel_select_pos = DMA_CSELR_C3S_Pos;
			break;

		case DMA1_Channel4_BASE:
		case DMA2_Channel4_BASE:
			peripheral->channel_select_pos = DMA_CSELR_C4S_Pos;
			break;

		case DMA1_Channel5_BASE:
		case DMA2_Channel5_BASE:
			peripheral->channel_select_pos = DMA_CSELR_C5S_Pos;
			break;

		case DMA1_Channel6_BASE:
		case DMA2_Channel6_BASE:
			peripheral->channel_select_pos = DMA_CSELR_C6S_Pos;
			break;

		case DMA1_Channel7_BASE:
		case DMA2_Channel7_BASE:
			peripheral->channel_select_pos = DMA_CSELR_C7S_Pos;
			break;
	}
}


void DMA_initializePeripheralConstants() {
	DMA_initWrapper(
		struct_DMA_ADC1,
		DMA1_Channel1,
		0b0
	);

	DMA_initWrapper(
		struct_DMA_ADC2,
		DMA1_Channel2,
		0b0
	);

	DMA_initWrapper(
		struct_DMA_ADC3,
		DMA1_Channel3,
		0b0
	);

	DMA_initWrapper(
		struct_DMA_SPI1_RX,
		DMA1_Channel2,
		0b0001
	);
	DMA_initWrapper(
		struct_DMA_SPI1_TX,
		DMA1_Channel3,
		0b0001
	);

	DMA_initWrapper(
		struct_DMA_SPI2_RX,
		DMA1_Channel4,
		0b0001
	);
	DMA_initWrapper(
		struct_DMA_SPI2_TX,
		DMA1_Channel5,
		0b0001
	);

	DMA_initWrapper(
		struct_DMA_SPI3_RX,
		DMA2_Channel1,
		0b0011
	);
	DMA_initWrapper(
		struct_DMA_SPI3_TX,
		DMA2_Channel2,
		0b0011
	);

	DMA_initWrapper(
		struct_DMA_SPI3_TX,
		DMA2_Channel2,
		0b0011
	);
	DMA_initWrapper(
		struct_DMA_SPI3_TX,
		DMA2_Channel2,
		0b0011
	);

	DMA_initWrapper(
		struct_DMA_USART1_RX,
		DMA1_Channel5,
		0b0010
	);
	DMA_initWrapper(
		struct_DMA_USART1_TX,
		DMA1_Channel4,
		0b0010
	);

	DMA_initWrapper(
		struct_DMA_USART2_RX,
		DMA1_Channel6,
		0b0010
	);
	DMA_initWrapper(
		struct_DMA_USART2_TX,
		DMA1_Channel7,
		0b0010
	);

	DMA_initWrapper(
		struct_DMA_LPUART1_RX,
		DMA2_Channel7,
		0b0100
	);
	DMA_initWrapper(
		struct_DMA_LPUART1_TX,
		DMA2_Channel6,
		0b0100
	);
}

DMAPeripheral* DMA_selectPeripheral(enum_DMAPeripherals selection) {
	switch (selection) {
		case SELECT_ADC1: return struct_DMA_ADC1;
		case SELECT_ADC2: return struct_DMA_ADC2;
		case SELECT_ADC3: return struct_DMA_ADC3;
		case SELECT_SPI1_RX: return struct_DMA_SPI1_RX;
		case SELECT_SPI1_TX: return struct_DMA_SPI1_TX;
		case SELECT_SPI2_RX: return struct_DMA_SPI2_RX;
		case SELECT_SPI2_TX: return struct_DMA_SPI2_TX;
		case SELECT_SPI3_RX: return struct_DMA_SPI3_RX;
		case SELECT_SPI3_TX: return struct_DMA_SPI3_TX;
		case SELECT_USART1_RX: return struct_DMA_USART1_RX;
		case SELECT_USART1_TX: return struct_DMA_USART1_TX;
		case SELECT_USART2_RX: return struct_DMA_USART2_RX;
		case SELECT_USART2_TX: return struct_DMA_USART2_TX;
		case SELECT_LPUART1_RX: return struct_DMA_LPUART1_RX;
		case SELECT_LPUART1_TX: return struct_DMA_LPUART1_TX;
	}

	return NULL; //We ought to have a result/error type...
}

/*
 * peripheral_defs.c
 *
 *  Created on: May 20, 2025
 */

#include "DMA/DMA.h"

DMAPeripheral dma_adc1, dma_adc2, dma_adc3,
	dma_spi1_rx, dma_spi1_tx,
	dma_spi2_rx, dma_spi2_tx,
	dma_spi3_rx, dma_spi3_tx;


DMAPeripheral* struct_DMA_ADC1 = &dma_adc1;
DMAPeripheral* struct_DMA_ADC2 = &dma_adc2;
DMAPeripheral* struct_DMA_ADC3 = &dma_adc3;

DMAPeripheral* struct_DMA_SPI1_RX = &dma_spi1_rx;
DMAPeripheral* struct_DMA_SPI1_TX = &dma_spi1_tx;

DMAPeripheral* struct_DMA_SPI2_RX = &dma_spi2_rx;
DMAPeripheral* struct_DMA_SPI2_TX = &dma_spi2_tx;

DMAPeripheral* struct_DMA_SPI3_RX = &dma_spi3_rx;
DMAPeripheral* struct_DMA_SPI3_TX = &dma_spi3_tx;

void DMA_initializePeripheralConstants() {
	struct_DMA_ADC1->channel = DMA1_Channel1;
	struct_DMA_ADC1->channel_select = DMA1_CSELR;
	struct_DMA_ADC1->channel_select_pos = DMA_CSELR_C1S_Pos;
	struct_DMA_ADC1->channel_select_value = 0b0;

	struct_DMA_ADC2->channel = DMA1_Channel2;
	struct_DMA_ADC2->channel_select = DMA1_CSELR;
	struct_DMA_ADC2->channel_select_pos = DMA_CSELR_C2S_Pos;
	struct_DMA_ADC2->channel_select_value = 0b0;

	struct_DMA_ADC3->channel = DMA1_Channel3;
	struct_DMA_ADC3->channel_select = DMA1_CSELR;
	struct_DMA_ADC3->channel_select_pos = DMA_CSELR_C3S_Pos;
	struct_DMA_ADC3->channel_select_value = 0b0;

	struct_DMA_SPI1_RX->channel = DMA1_Channel2;
	struct_DMA_SPI1_RX->channel_select = DMA1_CSELR;
	struct_DMA_SPI1_RX->channel_select_pos = DMA_CSELR_C2S_Pos;
	struct_DMA_SPI1_RX->channel_select_value = 0b0001;

	struct_DMA_SPI1_TX->channel = DMA1_Channel3;
	struct_DMA_SPI1_TX->channel_select = DMA1_CSELR;
	struct_DMA_SPI1_TX->channel_select_pos = DMA_CSELR_C3S_Pos;
	struct_DMA_SPI1_TX->channel_select_value = 0b0001;

	struct_DMA_SPI2_RX->channel = DMA1_Channel4;
	struct_DMA_SPI2_RX->channel_select = DMA1_CSELR;
	struct_DMA_SPI2_RX->channel_select_pos = DMA_CSELR_C4S_Pos;
	struct_DMA_SPI2_RX->channel_select_value = 0b0001;

	struct_DMA_SPI2_TX->channel = DMA1_Channel5;
	struct_DMA_SPI2_TX->channel_select = DMA1_CSELR;
	struct_DMA_SPI2_TX->channel_select_pos = DMA_CSELR_C5S_Pos;
	struct_DMA_SPI2_TX->channel_select_value = 0b0001;

	struct_DMA_SPI3_RX->channel = DMA2_Channel1;
	struct_DMA_SPI3_RX->channel_select = DMA2_CSELR;
	struct_DMA_SPI3_RX->channel_select_pos = DMA_CSELR_C1S_Pos;
	struct_DMA_SPI3_RX->channel_select_value = 0b0011;

	struct_DMA_SPI3_TX->channel = DMA2_Channel2;
	struct_DMA_SPI3_TX->channel_select = DMA2_CSELR;
	struct_DMA_SPI3_TX->channel_select_pos = DMA_CSELR_C2S_Pos;
	struct_DMA_SPI3_TX->channel_select_value = 0b0011;
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
	}

	return NULL;
}

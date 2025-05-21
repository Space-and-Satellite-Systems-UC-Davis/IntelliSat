/*
 * peripheral_defs.c
 *
 *  Created on: May 20, 2025
 */

#include "DMA/DMA.h"

//	DMA_ADC1 = (uint32_t) DMA1_Channel1,
//	DMA_ADC2 = (uint32_t) DMA1_Channel2,
//	DMA_ADC3 = (uint32_t) DMA1_Channel3,
//	DMA_SPI1_RX = (uint32_t) DMA1_Channel2,
//	DMA_SPI1_TX = (uint32_t) DMA1_Channel3,
//	DMA_SPI2_RX = (uint32_t) DMA1_Channel4,
//	DMA_SPI2_TX = (uint32_t) DMA1_Channel5,
//	DMA_SPI3_RX = (uint32_t) DMA2_Channel1,
//	DMA_SPI3_TX = (uint32_t) DMA2_Channel2

DMAPeripheral dma_adc1, dma_adc2, dma_adc3,
	dma_spi1_rx, dma_spi1_tx,
	dma_spi2_rx, dma_spi2_tx,
	dma_spi3_rx, dma_spi3_tx;


DMAPeripheral* DMA_ADC1 = &dma_adc1;
DMAPeripheral* DMA_ADC2 = &dma_adc2;
DMAPeripheral* DMA_ADC3 = &dma_adc3;

DMAPeripheral* DMA_SPI1_RX = &dma_spi1_rx;
DMAPeripheral* DMA_SPI1_TX = &dma_spi1_tx;

DMAPeripheral* DMA_SPI2_RX = &dma_spi2_rx;
DMAPeripheral* DMA_SPI2_TX = &dma_spi2_tx;

DMAPeripheral* DMA_SPI3_RX = &dma_spi3_rx;
DMAPeripheral* DMA_SPI3_TX = &dma_spi3_tx;

void DMA_initializePeripheralConstants() {
	DMA_ADC1->channel = DMA1_Channel1;
	DMA_ADC1->channel_select = 0b0;

	DMA_ADC2->channel = DMA1_Channel2;
	DMA_ADC2->channel_select = 0b0;

	DMA_ADC3->channel = DMA1_Channel3;
	DMA_ADC3->channel_select = 0b0;

	DMA_SPI1_RX->channel = DMA1_Channel2;
	DMA_SPI1_RX->channel_select = 0b0001;
	DMA_SPI1_TX->channel = DMA1_Channel3;
	DMA_SPI1_TX->channel_select = 0b0001;

	DMA_SPI2_RX->channel = DMA1_Channel4;
	DMA_SPI2_RX->channel_select = 0b0001;
	DMA_SPI2_TX->channel = DMA1_Channel5;
	DMA_SPI2_TX->channel_select = 0b0001;

	DMA_SPI3_RX->channel = DMA2_Channel1;
	DMA_SPI3_RX->channel_select = 0b0011;
	DMA_SPI3_TX->channel = DMA2_Channel2;
	DMA_SPI3_TX->channel_select = 0b0011;
}

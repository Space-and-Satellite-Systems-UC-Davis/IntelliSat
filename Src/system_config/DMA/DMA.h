#ifndef DMA_H
#define DMA_H

#include "stm32l476xx.h"
#include <globals.h>

// Only uncomment when SPI/UART are implemented
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

typedef struct DMAConfig {
	enum_DMAPeripherals selection; // Which peripheral we are working with
	uint32_t peripheral_addr; // Address of the peripheral data register
	uint32_t memory_addr; // Address of the memory
	uint16_t length; // How many units of data to read
	uint8_t pdata_size; // Number of bytes in peripheral. 1, 2, or 4
	uint8_t mdata_size; // Number of bytes in memory. 1, 2, or 4
	bool circular; // Whether DMA should read continuously
	bool peripheral_to_memory; // True if DMA should copy from periph. to mem.
	bool peripheral_increment; // Automatically increment peripheral pointer
	bool memory_increment; // Automatically increment memory pointer.
	bool transfer_interrupt; // Trigger interrupt on transfer complete?
	bool error_interrupt; // Trigger interrupt on transfer error?
} DMAConfig;

/**
 * Initialize the peripheral structs to be returned in dma_selectPeripheral.
 * Call upon board initialization.
 *
 * @returns None
 */
void dma_initializePeripheralConstants();

/**
 * Enable or disable the DMA channel
 *
 * @param  selection   Specific peripheral
 *
 * @returns requested peripheral
 */
DMAPeripheral* dma_selectPeripheral(enum_DMAPeripherals selection);

/**
 * Applies the config to the DMA channel chosen and enables it
 * NOTE: There is explicitly no way of "resuming" without reconfiguring
 * WARNING: If reconfiguring, disable both sides first.
 *
 * @param  config   See DMAConfig struct to learn the options
 *
 * @returns None
 */
void dma_configureAndEnableChannel(DMAConfig config);

/**
 * Disable the DMA channel
 * WARNING: You should disable DMA peripheral-side prior to disabling channel
 *
 * @param  selection   Specific peripheral. Finds channel by attached periph.
 *
 * @returns None
 */
void dma_disableChannel(enum_DMAPeripherals selection);

#endif 

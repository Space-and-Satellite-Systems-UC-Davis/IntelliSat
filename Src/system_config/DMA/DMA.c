#include "DMA/DMA.h"

void dma_configureAndEnableChannel(DMAConfig config) {
	DMAPeripheral* peripheral = dma_selectPeripheral(config.selection);
	DMA_Channel_TypeDef* channel_ptr = peripheral->channel;

	//Check what DMA controller we are working with
	//DMA1_Channel7_BASE is simply the largest address in DMA1 controller that may be passed
	if ((uint32_t)channel_ptr <= DMA1_Channel7_BASE) { //The channel is in DMA1
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	} else { //DMA2
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
	}

	//Reset all
	channel_ptr->CCR &= ~(
		  DMA_CCR_MEM2MEM_Msk
		| DMA_CCR_MSIZE_Msk
		| DMA_CCR_PSIZE_Msk
		| DMA_CCR_MINC_Msk
		| DMA_CCR_PINC_Msk
		| DMA_CCR_CIRC_Msk
		| DMA_CCR_DIR_Msk
		| DMA_CCR_EN_Msk
	);

	//Configure channel

	//Priority is explicitly not an option. We did not yet coordinate what gets what priority yet
	channel_ptr->CCR |= ( 0x1 << DMA_CCR_PL_Pos ); //0b01 Medium priority

	//Configure size of data
	switch (config.mdata_size) {
		case 1: channel_ptr->CCR |= ( (0b00) << DMA_CCR_MSIZE_Pos ); break; //0b00 8 bits
		case 2: channel_ptr->CCR |= ( (0b01) << DMA_CCR_MSIZE_Pos ); break; //0b01 16 bits
		case 4: channel_ptr->CCR |= ( (0b10) << DMA_CCR_MSIZE_Pos ); break; //0b10 32 bits
	}
	switch (config.pdata_size) {
		case 1: channel_ptr->CCR |= ( (0b00) << DMA_CCR_PSIZE_Pos ); break; //0b00 8 bits
		case 2: channel_ptr->CCR |= ( (0b01) << DMA_CCR_PSIZE_Pos ); break; //0b01 16 bits
		case 4: channel_ptr->CCR |= ( (0b10) << DMA_CCR_PSIZE_Pos ); break; //0b10 32 bits
	}

	if (config.peripheral_increment == true) { channel_ptr->CCR |= DMA_CCR_PINC; } //Increment peripheral
	if (config.memory_increment == true) { channel_ptr->CCR |= DMA_CCR_MINC; } //Increment memory

	if (config.peripheral_to_memory == true) {
		channel_ptr->CCR &= ~DMA_CCR_DIR; //Dir=0. Peripheral to memory
	} else {
		channel_ptr->CCR |= DMA_CCR_DIR; //Dir=1. Memory to peripheral
	}

	//Should DMA repeatedly read
	if (config.circular == true) { channel_ptr->CCR |= DMA_CCR_CIRC; }

	//Should an interrupt trigger upon transfer completion?
	if (config.transfer_interrupt == true) { channel_ptr->CCR |= DMA_CCR_TCIE; }

	//Should an interrupt trigger upon transfer error?
	if (config.error_interrupt == true) { channel_ptr->CCR |= DMA_CCR_TEIE; }

	//We can also configure it such that it triggers on half-transfer

	// Set src and dist
	//Not cast to pointer because pointer could be something other than uint32_t??
		// Other code I've seen does it like this and it appears to work
	channel_ptr->CMAR  = (uint32_t)config.memory_addr;
	channel_ptr->CPAR  = (uint32_t)config.peripheral_addr;
	channel_ptr->CNDTR = (uint16_t)config.length;

	// Turn it on
	dma_enableChannel(config.selection);
}

// Explicitly not a publicly exposed function because you should always
// reconfigure the channel. Effect of "resuming" is indeterminate.
void dma_enableChannel(enum_DMAPeripherals selection) {
	DMAPeripheral* peripheral = dma_selectPeripheral(selection);
	DMA_Channel_TypeDef* channel_ptr = peripheral->channel;

	//Enable without resetting registers
	peripheral->channel_select->CSELR |=
			(peripheral->channel_select_value) << peripheral->channel_select_pos;
	channel_ptr->CCR |= DMA_CCR_EN;
}

void dma_disableChannel(enum_DMAPeripherals selection) {
	DMAPeripheral* peripheral = dma_selectPeripheral(selection);
	DMA_Channel_TypeDef* channel_ptr = peripheral->channel;

	//Disable without resetting registers
	peripheral->channel_select->CSELR &=
			~(peripheral->channel_select_value) << peripheral->channel_select_pos;
	channel_ptr->CCR &= ~(DMA_CCR_EN);
}


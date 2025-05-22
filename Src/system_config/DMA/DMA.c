#include "DMA/DMA.h"

void configure_channel(DMAConfig config) {
	DMAPeripheral* peripheral = DMA_selectPeripheral(config.selection);
	DMA_Channel_TypeDef* channel_ptr = peripheral->channel;

	//Check what DMA controller we are working with
	//DMA1_CSELR_BASE is simply the largest address in DMA1 controller
	if ((uint32_t)channel_ptr <= DMA1_CSELR_BASE) { //The channel is in DMA1
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	} else {
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
	channel_ptr->CCR |= ( 0x1 << DMA_CCR_PL_Pos ); //0b01 Medium priority
	channel_ptr->CCR |= ( 0x1 << DMA_CCR_MSIZE_Pos ); //0b01 16 bits data
	channel_ptr->CCR |= ( 0x1 << DMA_CCR_PSIZE_Pos ); //0b01 16 bits data
//	config.channel->CCR |= DMA_CCR_MINC; //Increment memory
	channel_ptr->CCR &= ~DMA_CCR_DIR; //Dir=0. Peripheral to memory

	//Could probablly shorten to config.channel->CCR |= (DMA_CCR_CIRC * config.circular)
	//But this also reads easier
	if (config.circular == true) {
		channel_ptr->CCR |= DMA_CCR_CIRC;
	}

	// Set src and dist. Currently set to peripheral -> memory
	//Not cast to pointer because pointer could be something other than uint32_t??
	//^^^If this comment is still here I haven't tested what goes wrong otherwise
	channel_ptr->CMAR  = (uint32_t)config.memory_addr;
	channel_ptr->CPAR  = (uint32_t)config.peripheral_addr;
	channel_ptr->CNDTR = (uint16_t)config.length;

	if (config.on_by_default) {
		dma_enable_channel(config.selection);
	}
}


//For use by peripherals
void dma_enable_channel(enum_DMAPeripherals selection) {
	DMAPeripheral* peripheral = DMA_selectPeripheral(selection);
	DMA_Channel_TypeDef* channel_ptr = peripheral->channel;

	//Enable without resetting registers
	peripheral->channel_select->CSELR |=
			(peripheral->channel_select_value) << peripheral->channel_select_pos;
	channel_ptr->CCR |= DMA_CCR_EN;
}

void dma_disable_channel(enum_DMAPeripherals selection) {
	DMAPeripheral* peripheral = DMA_selectPeripheral(selection);
	DMA_Channel_TypeDef* channel_ptr = peripheral->channel;

	//Disable without resetting registers
	peripheral->channel_select->CSELR &=
			~(peripheral->channel_select_value) << peripheral->channel_select_pos;
	channel_ptr->CCR &= ~(DMA_CCR_EN);
}


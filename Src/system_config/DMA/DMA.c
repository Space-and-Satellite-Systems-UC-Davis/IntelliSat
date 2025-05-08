#include "DMA/DMA.h"

void configure_channel(DMAConfig config) {
	//Reset all
	config.channel->CCR &= ~(
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
	config.channel->CCR |=  (
			( 0x1 << DMA_CCR_PL_Pos ) | //0b01 Medium priority
			( 0x1 << DMA_CCR_MSIZE_Pos ) | //0b01 16 bits data
			( 0x1 << DMA_CCR_PSIZE_Pos ) | //0b01 16 bits data
			DMA_CCR_MINC | //Increment memory
			~DMA_CCR_DIR); //Dir=0. Peripheral to memory

	if (config.circular == true) {
		config.channel->CCR |= DMA_CCR_CIRC;
	}


	// Set src and dist. Currently set to peripheral -> memory
	//Not cast to pointer because pointer could be something other than uint32_t??
	//^^^If this comment is still here I haven't tested what goes wrong otherwise
	config.channel->CMAR  = (uint32_t)config.memory_addr;
	config.channel->CPAR  = (uint32_t)ADC3->DR;
	config.channel->CNDTR = (uint16_t)config.length;

	config.channel->CCR |= ( DMA_CCR_EN ); //Enable

	//Check what DMA controller we are working with
	//DMA1_CSELR_BASE is simply the largest address in DMA1 controller
	if ((uint32_t)config.channel <= DMA1_CSELR_BASE) { //The channel is in DMA1
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	} else {
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
	}

}

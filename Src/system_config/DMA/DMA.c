#include "DMA/DMA.h"

void configure_channel(
		DMA_Channel_TypeDef* channel,
		uint32_t* peripheral_addr,
		uint32_t* memory_addr,
		size_t length
) {
	//Reset all
	channel->CCR &= ~(
		  DMA_CCR_MEM2MEM_Msk
		| DMA_CCR_MSIZE_Msk
		| DMA_CCR_PSIZE_Msk
		| DMA_CCR_MINC_Msk
		| DMA_CCR_PINC_Msk
		| DMA_CCR_CIRC_Msk
		| DMA_CCR_DIR_Msk
		| DMA_CCR_EN_Msk
	);



	//Check what DMA controller we are working with
	if (channel <= DMA1_CSELR_BASE) { //The channel is in DMA1
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	} else {
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
	}
}

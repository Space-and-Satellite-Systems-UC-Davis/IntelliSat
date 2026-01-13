#include "DMA/DMA.h"
#include <string.h>

//Template function to create USART TX DMAConfig

DMAConfig USART_TX_Config(enum_DMAPeripherals selection, uint32_t memory_addr, uint16_t length) {
	DMAConfig config;

	config.selection = selection;
	if (selection == SELECT_USART2_TX) {
		config.peripheral_addr = (uint32_t)&USART2->TDR;
	} else if (selection == SELECT_USART1_TX) {
		config.peripheral_addr = (uint32_t)&USART1->TDR;
	}
	config.memory_addr = memory_addr;
	config.length = length;
	config.pdata_size = 1;
	config.mdata_size = 1;
	config.circular = false;
	config.peripheral_to_memory = false;
	config.peripheral_increment = false;
	config.memory_increment = true;
	config.transfer_interrupt = true;

	return config;
}

DMAConfig USART_RX_Config(enum_DMAPeripherals selection, uint32_t memory_addr, uint16_t length) {
	DMAConfig config;

	config.selection = selection;
	if (selection == SELECT_USART2_RX) {
		config.peripheral_addr = (uint32_t)&USART2->RDR;
	} else if (selection == SELECT_USART1_RX) {
		config.peripheral_addr = (uint32_t)&USART1->RDR;
	}
	config.memory_addr = memory_addr;
	config.length = length;
	config.pdata_size = 1;
	config.mdata_size = 1;
	config.circular = false;
	config.peripheral_to_memory = true;
	config.peripheral_increment = false;
	config.memory_increment = true;
	config.transfer_interrupt = true;

	return config;
}


void configure_channel(DMAConfig config) {
	DMAPeripheral* peripheral = DMA_selectPeripheral(config.selection);
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

	//Could probablly shorten to config.channel->CCR |= (DMA_CCR_CIRC * config.circular)
	//But this also reads easier
	if (config.circular == true) { channel_ptr->CCR |= DMA_CCR_CIRC; }

	if (config.transfer_interrupt == true) { channel_ptr->CCR |= DMA_CCR_TCIE; }

	// Set src and dist
	//Not cast to pointer because pointer could be something other than uint32_t??
	//^^^If this comment is still here I haven't tested what goes wrong otherwise
	channel_ptr->CMAR  = (uint32_t)config.memory_addr;
	channel_ptr->CPAR  = (uint32_t)config.peripheral_addr;
	channel_ptr->CNDTR = (uint16_t)config.length;
}

//Actually turn the DMA on
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

/*
Transmit bytes over USART1 or USART2 using DMA, selection must be USARTx_TX
*/
void usart_transmitBytesDMA(uint8_t message[], enum_DMAPeripherals selection, bool circular)
{
	if (selection == SELECT_USART2_TX) 
	{
		//Enable USART2 clock
		RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN_Msk;
		DMAConfig config = USART_TX_Config(selection, (uint32_t)message, strlen((char*)message));
		config.circular = circular;
		configure_channel(config);

		//Enable DMA transmitter
		USART2->CR3 |= USART_CR3_DMAT;
		//Enable transmitter and USART
		USART2->CR1 |= USART_CR1_TE | USART_CR1_UE;
		dma_enable_channel(config.selection);
	}
	else if (selection == SELECT_USART1_TX) 
	{
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN_Msk;
		DMAConfig config = USART_TX_Config(selection, (uint32_t)message, strlen((char*)message));
		config.circular = circular;
		configure_channel(config);
		USART1->CR3 |= USART_CR3_DMAT;
		USART1->CR1 |= USART_CR1_TE | USART_CR1_UE;
		dma_enable_channel(config.selection);
	}
	else return; //Unsupported USART

}

//Receive bytes over USART1 or USART2 using DMA, selection must be USARTx_RX
void usart_recieveBytesDMA(uint8_t buffer[], uint16_t size, enum_DMAPeripherals selection)
{
	if (selection == SELECT_USART2_RX) 
	{
		RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN_Msk;
		DMAConfig config = USART_RX_Config(selection, (uint32_t)buffer, size);
		configure_channel(config);
		USART2->CR3 |= USART_CR3_DMAR;
		USART2->CR1 |= USART_CR1_RE | USART_CR1_UE;
		dma_enable_channel(config.selection);
	}
	else if (selection == SELECT_USART1_RX) 
	{
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN_Msk;
		DMAConfig config = USART_RX_Config(selection, (uint32_t)buffer, size);
		configure_channel(config);
		USART1->CR3 |= USART_CR3_DMAR;
		USART1->CR1 |= USART_CR1_RE | USART_CR1_UE;
		dma_enable_channel(config.selection);
	}
	else return; //Unsupported USART
}


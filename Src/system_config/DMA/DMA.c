#include "DMA/DMA.h"
#include <print_scan.h>


volatile bool usart1_tx_ready = true; //Flag to indicate if USART1 is ready for another transmission. Set in DMA1_Channel4_IRQHandler
volatile bool usart1_rx_ready = true; //Flag to indicate if USART1 has received data. Set in DMA1_Channel5_IRQHandler


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
    config.circular = true;
    config.peripheral_to_memory = true;
    config.peripheral_increment = false;
    config.memory_increment = true;
    config.transfer_interrupt = true;

    return config;
}


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

// Actually turn the DMA on
void dma_enable_channel(enum_DMAPeripherals selection) {
    DMAPeripheral* peripheral = DMA_selectPeripheral(selection);
    DMA_Channel_TypeDef* channel_ptr = peripheral->channel;

    // clear the 4-bit CSELR field for this channel first
    peripheral->channel_select->CSELR &= ~(0xFU << peripheral->channel_select_pos);
    
    // set the new CSELR mapping
    peripheral->channel_select->CSELR |= 
            (peripheral->channel_select_value) << peripheral->channel_select_pos;

    // clear ALL pending DMA flags before enabling.
    if ((uint32_t)channel_ptr <= DMA1_Channel7_BASE) {
        DMA1->IFCR = (0xFU << peripheral->channel_select_pos);
    } else {
        DMA2->IFCR = (0xFU << peripheral->channel_select_pos);
    }

    // Enable the channel
    channel_ptr->CCR |= DMA_CCR_EN;
}

void dma_disable_channel(enum_DMAPeripherals selection) {
    DMAPeripheral* peripheral = DMA_selectPeripheral(selection);
    DMA_Channel_TypeDef* channel_ptr = peripheral->channel;

    // clear the CSELR mapping without corrupting other channels
    peripheral->channel_select->CSELR &= ~(0xFU << peripheral->channel_select_pos);
    
    // Disable the channel
    channel_ptr->CCR &= ~(DMA_CCR_EN);
}


/*
Transmit bytes over USART1 or USART2 using DMA, selection must be USARTx_TX
*/
bool usart_transmitBytesDMA(enum_DMAPeripherals selection, const uint8_t *tx_buffer, uint16_t length)
{
	USART_TypeDef* usart_ptr;
	if (selection == SELECT_USART2_TX) 
	{
		if (!(RCC->APB1ENR1 & RCC_APB1ENR1_USART2EN_Msk)) RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN_Msk;
		usart_ptr = USART2;
	} 
	else if (selection == SELECT_USART1_TX) 
	{
		if (!(RCC->APB2ENR & RCC_APB2ENR_USART1EN_Msk)) RCC->APB2ENR |= RCC_APB2ENR_USART1EN_Msk;
		usart_ptr = USART1;
	} 
	else return false; //Unsupported USART

	// get config
	DMAConfig config = USART_TX_Config(selection, (uint32_t)tx_buffer, length);
	DMAPeripheral* peripheral = DMA_selectPeripheral(config.selection);
	
	// leave if the DMA channel is already active
	if (peripheral->channel->CCR & DMA_CCR_EN_Msk) return false; 


	while (!usart1_tx_ready) {
    }

    usart1_tx_ready = false;

	configure_channel(config);

	// clear usart transfer complete flag
	usart_ptr->ICR |= USART_ICR_TCCF;
	// enable DMA transmitter and USART
	usart_ptr->CR3 |= USART_CR3_DMAT;
	usart_ptr->CR1 |= USART_CR1_TE | USART_CR1_UE;
	
	dma_enable_channel(config.selection);
	return true;
}

//Receive bytes over USART1 or USART2 using DMA, selection must be USARTx_RX
bool usart_receiveBytesDMA(enum_DMAPeripherals selection, uint8_t *rx_buffer, uint16_t length)
{
	USART_TypeDef* usart_ptr;
	if (selection == SELECT_USART2_RX) 
	{
		if (!(RCC->APB1ENR1 & RCC_APB1ENR1_USART2EN_Msk)) RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN_Msk;
		usart_ptr = USART2;
	} 
	else if (selection == SELECT_USART1_RX) 
	{
		if (!(RCC->APB2ENR & RCC_APB2ENR_USART1EN_Msk)) RCC->APB2ENR |= RCC_APB2ENR_USART1EN_Msk;
		usart_ptr = USART1;
	} 
	else return false; //Unsupported USART

	// 1. get config
	DMAConfig config = USART_RX_Config(selection, (uint32_t)rx_buffer, length);
	DMAPeripheral* peripheral = DMA_selectPeripheral(config.selection);
	
	// 2. leave if the DMA channel is already active
	if (peripheral->channel->CCR & DMA_CCR_EN_Msk) return false; 


	configure_channel(config);

	// 4. clear usart overrun flag (in case it was set, otherwise the DMA won't work)
	usart_ptr->ICR |= USART_ICR_ORECF;
	// enable DMA receiver and USART
	
	dma_enable_channel(config.selection);

	usart_ptr->CR3 |= USART_CR3_DMAR;
	usart_ptr->CR1 |= USART_CR1_RE | USART_CR1_UE;
	return true;
}

void dma_init() {
	DMA_initializePeripheralConstants();
	NVIC_EnableIRQ(DMA1_Channel4_IRQn); //Enable the IRQ for the channel we will be using for USART1 TX. We can enable other IRQs when we use other channels
	NVIC_EnableIRQ(DMA1_Channel5_IRQn); //Enable the IRQ for the channel we will be using for USART1 RX.
}

void dma_test(void) {
    printMsg("Starting DMA Test...\n");

    const uint32_t timeout_ms = DEFAULT_TIMEOUT_MS;
    const uint8_t message[] = "dma loopback test\n";
    uint8_t rx_buf[sizeof(message)] = {0};

    // Clear stale state
    usart1_tx_ready = true;
    usart1_rx_ready = false;

    if (!usart_receiveBytesDMA(SELECT_USART1_RX, rx_buf, sizeof(rx_buf))) {
        printMsg("DMA RX start failed\n");
        return;
    }

    if (!usart_transmitBytesDMA(SELECT_USART1_TX, message, sizeof(message))) {
        printMsg("DMA TX start failed\n");
        return;
    }

    uint64_t start = getSysTime();
    while (!usart1_tx_ready && !is_time_out(start, timeout_ms)) {
    }
    if (!usart1_tx_ready) {
        printMsg("DMA TX timeout\n");
        return;
    }

    start = getSysTime();
    while (!usart1_rx_ready && !is_time_out(start, timeout_ms)) {
    }
    if (!usart1_rx_ready) {
        printMsg("DMA RX timeout\n");
        return;
    }

    bool match = true;
    for (uint16_t i = 0; i < sizeof(message); i++) {
        if (rx_buf[i] != message[i]) { match = false; break; }
    }

    if (!match) {
        printMsg("DMA RX mismatch\n");
        return;
    }

    printMsg("DMA loopback OK\n");
}


//DMA1_Channel4_IRQHandler is the IRQ handler for USART1 TX. 
void DMA1_Channel4_IRQHandler(void) {
    if (DMA1->ISR & DMA_ISR_TCIF4) {
        DMA1->IFCR |= DMA_IFCR_CTCIF4;
        usart1_tx_ready = true;
    }
}

//DMA1_Channel5_IRQHandler is the IRQ handler for USART1 RX.
void DMA1_Channel5_IRQHandler(void) {
	if (DMA1->ISR & DMA_ISR_TCIF5) {
		DMA1->IFCR |= DMA_IFCR_CTCIF5;
		usart1_rx_ready = true;
	}
}



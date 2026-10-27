#ifndef CRC_TEST_UART_H
#define CRC_TEST_UART_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct { int unused; } USART_TypeDef;
void usart_transmitBytes(USART_TypeDef *bus, uint8_t *bytes, int count);
int usart_receiveBytes(USART_TypeDef *bus, uint8_t *out, uint16_t size);
bool packet_available(void);
#endif

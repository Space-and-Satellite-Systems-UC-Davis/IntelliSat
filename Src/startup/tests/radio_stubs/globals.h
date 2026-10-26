#ifndef RADIO_TEST_GLOBALS_H
#define RADIO_TEST_GLOBALS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct { unsigned unused; } USART_TypeDef;
extern USART_TypeDef test_usart;
#define USART1 (&test_usart)
#endif

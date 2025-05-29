#include <MGTINTERCOM/mgt_intercom.h>
#include <print_scan.h>

void testFunction_mgtIntercom() {
    // usart_init(USART1, 9600);
    while (1) {
        usart_transmitBytes(USART2, "HELP!!!");
        // printMsg("!"); // 0x40008000
        delay_ms(1000);
    }
}

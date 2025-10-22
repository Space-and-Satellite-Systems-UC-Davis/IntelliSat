#include <MGTINTERCOM/mgt_intercom.h>
#include <print_scan.h>

void testFunction_mgtIntercom() {
    mgt_intercom_init();
    while (1) {
        delay_ms(100);
        float code = mgt_intercom_get_current(1);
        char data[100];
        sprintf(data, "%f", code);
        printMsg(data);
        delay_ms(100);
        char code2 = mgt_intercom_set_coil_percent(1, 3, 10) + '0';
        printMsg(&code2);
        delay_ms(100);
        code2 = mgt_intercom_shutdown_all() + '0';
        printMsg(&code2);
        delay_ms(100);
        code2 = mgt_intercom_shutdown_timer(6) + '0';
        printMsg(&code2);        
        printMsg("Cycle\r\n");
    }
}

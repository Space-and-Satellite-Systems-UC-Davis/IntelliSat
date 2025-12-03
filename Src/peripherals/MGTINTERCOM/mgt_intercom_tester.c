#include <MGTINTERCOM/mgt_intercom.h>
#include <print_scan.h>
#include <string.h>

void testFunction_mgtIntercom() {
    mgt_intercom_init();
    while (1) {
        delay_ms(100);
        float code = mgt_intercom_get_current(1);
        printMsg("%f\r\n", code);
        delay_ms(100);
        int code2 = mgt_intercom_set_coil_percent(1, 3, 10);
        printMsg("%d\r\n", code2);
        delay_ms(100);
        code2 = mgt_intercom_shutdown_all();
        printMsg("%d\r\n", code2);
        delay_ms(100);
        code2 = mgt_intercom_shutdown_timer(6);
        printMsg("%d\r\n", code2);        
        printMsg("Cycle\r\n");
    }
}

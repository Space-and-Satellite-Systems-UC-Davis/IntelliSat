#include <MGTINTERCOM/mgt_intercom.h>
#include <print_scan.h>

void testFunction_mgtIntercom() {
    mgt_intercom_init();
    while (1) {
        delay_ms(100);
        mgt_intercom_get_current(1);
        mgt_intercom_set_coil_percent(1, 3, 10);
        mgt_intercom_shutdown_all();
        mgt_intercom_shutdown_timer(6);        
    }
}

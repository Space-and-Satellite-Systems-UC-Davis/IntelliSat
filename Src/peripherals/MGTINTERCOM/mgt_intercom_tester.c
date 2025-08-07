#include <MGTINTERCOM/mgt_intercom.h>
#include <print_scan.h>

void testFunction_mgtIntercom() {
    PCPDevice pcp;
    mgt_intercom_init(&pcp);
    // uint8_t chunk[8] = {'B'};
    mgt_intercom_get_current(&pcp, 1);
    mgt_intercom_set_coil_percent(&pcp, 1, 3, 10);
    mgt_intercom_shutdown_all(&pcp);
    mgt_intercom_shutdown_timer(&pcp, 6);
    uint8_t data[256];
    while (1) {
        for (int i = 0; i < 1000; i++) {
        pcp_retransmit(&pcp);
        }
        mgt_intercom_get_current(&pcp, 1);
        mgt_intercom_set_coil_percent(&pcp, 1, 3, 10);
        mgt_intercom_shutdown_all(&pcp);
        mgt_intercom_shutdown_timer(&pcp, 6);        
    }
}

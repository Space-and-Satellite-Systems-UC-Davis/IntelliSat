#include <MGTINTERCOM/mgt_intercom.h>
#include <print_scan.h>
#include <string.h>

void testFunction_mgtIntercom() {
    mgt_intercom_init();
    printMsg("START");
    int i = 5;
    
    while(1){
        int code1 = mgt_intercom_turnOnTimer(0);
        printMsg("TurnOn: %d\r\n", code1);
        delay_ms(1000);
        int code2 = mgt_intercom_setCoilPercent(0, (i)%100);
        printMsg("SetCoilPercent: to %d,  %d\r\n", i, code2);
        i+=25;

        // float current = mgt_intercom_getCurrent(0);
        // printMsg("Got current = %f\r\n", current);
        delay_ms(5000);
        // mgt_intercom_shutdownTimer(0);
        // mgt_intercom_shutdownTimer(0);
        code2 = mgt_intercom_shutdownTimer(0);//mgt_intercom_shutdownAll();
        printMsg("Shutdown: %d\r\n", code2);
        delay_ms(2000);
    }
    // while (1) {
    //     mgt_intercom_turnOnTimer(0);
    //     // float code = mgt_intercom_getCurrent(0);
    //     // printMsg("Got current = %f\r\n", code);
    //     int code2 = mgt_intercom_setCoilPercent(0, 10+i);
    //     i+=20;
    //     printMsg("Code2 percent for %d: %d\r\n", i%100, code2);
    //     delay_ms(1000);
    //     code2 = mgt_intercom_shutdownAll();
    //     printMsg("Shutdown: %d\r\n", code2);
    //     delay_ms(1000);
    //     code2 = mgt_intercom_turnOnTimer(0);
    //     printMsg("Turn on: %d\r\n", code2);
    //     delay_ms(5000);
    //     code2 = mgt_intercom_shutdownTimer(0);
    //     printMsg("Shut down: %d\r\n", code2);        
    //     printMsg("Cycle\r\n");
    // }
}

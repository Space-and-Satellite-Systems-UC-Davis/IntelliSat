#include <MGTINTERCOM/mgt_intercom.h>
#include <print_scan.h>
#include <string.h>

#define TEST_SPEED 0
#define TEST_FUNCTIONALITY 1

void testFunction_mgtIntercom() {
    mgt_intercom_init();
    int testtype = TEST_SPEED;

    if(testtype == TEST_SPEED){
        int i = 5;
        mgt_intercom_turnOnTimer(0);
        while(1){
            mgt_intercom_setCoilPercent(0, (i + 5)%100);
            i+=25;
            printMsg("GET CURRENT: ");
            float code = mgt_intercom_getCurrent(0);
            printMsg("%f\r\n", code);
        }
    }else if(testtype == TEST_FUNCTIONALITY){
        int count = 0;
        while (1) {
            printMsg("TURN ON: ");
            int code2 = mgt_intercom_turnOnTimer(0);
            printMsg("%d\r\n", code2);
            delay_ms(1000);

            printMsg("GET CURRENT: ");
            float code = mgt_intercom_getCurrent(0);
            printMsg("%f\r\n", code);
            delay_ms(1000);

            printMsg("SET COIL to %d: ", (10 + count * 23) % 100);
            code2 = mgt_intercom_setCoilPercent(0, (10+(count++)*23)%100);
            printMsg("%d\r\n", code2);
            delay_ms(1000);

            printMsg("SET LOW: ");
            code2 = mgt_intercom_setCoilDirection(0, LOW);
            printMsg("%d\r\n" , code2);
            delay_ms(1000);

            printMsg("SET HIGH: ");
            code2 = mgt_intercom_setCoilDirection(0, HIGH);
            printMsg("%d\r\n" , code2);
            delay_ms(1000);

            printMsg("SHUT DOWN ALL: ");
            code2 = mgt_intercom_shutdownAll();
            printMsg("%d\r\n", code2);
            delay_ms(1000);

            printMsg("TURN ON: ");
            code2 = mgt_intercom_turnOnTimer(0);
            printMsg("%d\r\n", code2);
            delay_ms(1000);

            printMsg("TURN OFF: ");
            code2 = mgt_intercom_shutdownTimer(0);
            printMsg("%d\r\n", code2);        
            delay_ms(1000);

            printMsg("Cycle\r\n");
        }
    }
}

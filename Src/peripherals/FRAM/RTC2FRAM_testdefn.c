#include "FRAM.h"
#include "print_scan.h"

void testFunction_RTC2FRAM() {
    printMsg("\r\n-------------\n\rBegin RTC2FRAM testing suite\n\r");
    FRAM_writeDWord(40, 0x022fa11a);

    if (0x022fa11a != FRAM_readDWord(40))
    printMsg("FRAM no workie :(\n\r");

    printMsg("%d boots\n\r", FRAM_getBootCounter());
    printMsg("first time: %c\n\r", FRAM_isFirstTime() ? 'T' : 'F');
    FRAM_writeDWord(FRAM_ADCS_VARIABLE_ADDR, 0);

    if (FRAM_readFromADCSVariable(4))
    printMsg("ADCS variable module nullifier nonfunctional\n\r");

    FRAM_writeToADCSVariable(true, 4);

    if (!FRAM_readFromADCSVariable(4))
    printMsg("ADCS write broken\n\r");
}

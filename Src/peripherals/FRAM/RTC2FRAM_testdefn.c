#include "FRAM.h"
#include "print_scan.h"

void testFunction_RTC2FRAM() {
    FRAM_writeDWord(40, 0x022fa11a);
    if (0x022fa11a != FRAM_readDWord(40))
    while (true) printMsg("FRAM no workie :(");
    printMsg("%d boots", FRAM_getBootCounter());
    printMsg("first time: %c", FRAM_isFirstTime() ? 'T' : 'F');
    FRAM_writeDWord(FRAM_ADCS_VARIABLE_ADDR, 0);
    if (FRAM_readFromADCSVariable(4))
    while (true) printMsg("ADCS variable module nullifier nonfunctional");
    FRAM_writeToADCSVariable(true, 4);
    if (!FRAM_readFromADCSVariable(4))
    while (true) printMsg("ADCS write broken");
}

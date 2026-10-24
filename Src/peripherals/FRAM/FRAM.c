#include "FRAM.h"
#include "MB85RS256B.h"

uint32_t FRAM_readDWord(uint16_t address) {
    uint32_t out;
    FRAM_readData(address, &out, FRAM_BYTES_PER_DWORD);
    return out;
}

bool FRAM_writeDWord(uint16_t address, uint32_t value) {
	FRAM_writeEnable();
    return FRAM_writeData(address, &value, FRAM_BYTES_PER_DWORD);
	FRAM_writeDisable();
}

uint32_t FRAM_getBootCounter() {
	return FRAM_readDWord(FRAM_BOOT_COUNTER_ADDR);
}

void FRAM_increment_boot_counter() {
	FRAM_writeEnable();
	FRAM_writeDWord(FRAM_BOOT_COUNTER_ADDR, FRAM_readDWord(FRAM_BOOT_COUNTER_ADDR) + 1);
	FRAM_writeDisable();
}

bool FRAM_isFirstTime() {
	int boot_counter = FRAM_getBootCounter();
	if (boot_counter == 0 || boot_counter == 1) {
		return true;
	} else {
		return false;
	}
}

bool FRAM_readFromADCSVariable(SensorOffset offset) {
	return (FRAM_readDWord(FRAM_ADCS_VARIABLE_ADDR) >> (uint32_t) offset) & 0b1;
}

void FRAM_writeToADCSVariable(bool status, SensorOffset offset) {
	FRAM_writeEnable();

	uint32_t variables = FRAM_readDWord(FRAM_ADCS_VARIABLE_ADDR);

	variables &= ~(1 << (uint32_t) offset);
	variables |= (status << (uint32_t) offset);

	FRAM_writeDWord(FRAM_ADCS_VARIABLE_ADDR, variables);

	FRAM_writeDisable();
}

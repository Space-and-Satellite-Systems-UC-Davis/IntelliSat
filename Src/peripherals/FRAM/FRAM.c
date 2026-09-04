#include "FRAM.h"
#include "MB85RS256B.h"

uint32_t FRAM_readDWord(uint16_t address) {
    uint32_t buffer[FRAM_PAGE_SIZE_DWORDS];
    FRAM_readData(address / FRAM_PAGE_SIZE_BYTES * FRAM_PAGE_SIZE_BYTES, buffer);
    return buffer[address % FRAM_PAGE_SIZE_DWORDS];
}

bool FRAM_writeDWord(uint16_t address, uint32_t value) {
    uint32_t buffer[FRAM_PAGE_SIZE_DWORDS];
    if (!FRAM_readData(address / FRAM_PAGE_SIZE_BYTES * FRAM_PAGE_SIZE_BYTES, buffer)) return false;
    buffer[address % FRAM_PAGE_SIZE_DWORDS] = value;
    return FRAM_writeData(address, buffer, FRAM_PAGE_SIZE_BYTES);
}

uint32_t FRAM_getBootCounter() {
	return FRAM_readDWord(FRAM_BOOT_COUNTER_ADDR);
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
	uint32_t variables = FRAM_readDWord(FRAM_ADCS_VARIABLE_ADDR);

	variables &= ~(1 << (uint32_t) offset);
	variables |= (status << (uint32_t) offset);

	FRAM_writeDWord(FRAM_ADCS_VARIABLE_ADDR, variables);
}
#define FRAM_BOOT_COUNTER_ADDR 0
#define FRAM_ADCS_VARIABLE_ADDR 4 // boot counter and ADCS variable register are 4 bytes wide
#define FRAM_PAGE_SIZE_DWORDS 64 // 256 bytes (8 bit) is 128 words (16 bit) or 64 double-words (32 bit)

uint32_t FRAM_readDWord(uint16_t address);

bool FRAM_writeDWord(uint16_t address, bool value);

/**
 * Reads BKP0 to tell how many times we booted, incremented by rtc_config
 *
 * @returns true if it's the first time being on for the board.
 */
uint32_t FRAM_getBootCounter();

/**
 * Reads BKP0 to tell whether it's the first time this board was turned on.
 * NOTE: Not reliable before rtc_configure updates the flag.
 *
 * @returns true if it's the first time being on for the board.
 */
bool FRAM_isFirstTime();

/**
 * Read bit representing sensor status in BKP1
 *
 * @param offset	Which bit to modify out of 32
 *
 * @returns status of sensor
 */
bool FRAM_readFromADCSVariable(SensorOffset offset);

/**
 * Stores bit representing sensor status in BKP1
 *
 * @param status 	Whether the sensor is active or not
 * @param offset  	Which bit to modify out of 32
 *
 */
void FRAM_writeToADCSVariable(bool status, SensorOffset offset);
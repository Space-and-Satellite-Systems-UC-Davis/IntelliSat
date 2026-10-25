/**
 * @file QSPI_testing.c
 *
 * @brief Low-level checks that the QSPI link really reaches the W25Q128JV
 * FLASH. Run as TEST_ID 33.
 *
 * Talks to the chip with raw qspi_setCommand() / qspi_sendCommand() calls
 * instead of the FLASH driver, whose flash_wait() has no timeout. Every wait
 * here is bounded, so a missing or miswired chip prints FAIL instead of
 * hanging.
 *
 * The tests run in order and build on each other: the erase test leaves
 * TEST_SECTOR blank for the pattern and overwrite tests.
 *
 * Destructive: erases TEST_SECTOR and the sectors used by the address walk,
 * including sectors 0 and 1, which hold the littlefs superblocks. The next
 * boot's fs_mount() finds no filesystem and formats the FLASH.
 *
 * @author Li, Chun Ho (lchli@ucdavis.edu)
 * @date 10/24/2026
 */

#include "W25Q128JV.h"
#include <print_scan.h>
#include <string.h>

#define QSPI_NUM_TESTS 7

// Not a power of two, so it stays clear of the address walk's sectors.
#define TEST_SECTOR 100
#define TEST_PAGE (TEST_SECTOR * FLASH_PAGES_PER_SECTOR)

#define JEDEC_ID 0x9F
#define READ_REGISTER_THREE 0x15

#define SR1_BUSY (1 << 0)
#define SR1_WEL (1 << 1)
#define SR1_BP (0x7 << 2)
#define SR2_CMP (1 << 6)
#define SR2_SUS (1 << 7)
#define SR3_WPS (1 << 2)

// W25Q128JV datasheet maximums.
#define ERASE_MAX_US 400000
#define PROGRAM_MAX_US 3000

extern int core_Hz; // from core_config.h

static uint8_t expected[FLASH_PAGE_SIZE];
static uint8_t actual[FLASH_PAGE_SIZE];

/**
 * Runs one single-line command. Pass hasAddress false for commands with no
 * address phase, and len 0 for commands with no data phase.
 */
static bool command(uint8_t instruction, bool hasAddress, uint32_t address,
                    uint8_t *data, uint32_t len, bool r_or_w) {
  bool ok = qspi_setCommand(
      r_or_w == QSPI_READ ? QSPI_CCR_FMODE_INDIRECT_READ
                          : QSPI_CCR_FMODE_INDIRECT_WRITE,
      QSPI_CCR_IMODE_1_INSTRUCTION_LINES,
      hasAddress ? QSPI_CCR_ADMODE_1_ADDRESS_LINES
                 : QSPI_CCR_ADMODE_0_ADDRESS_LINES,
      0, 0,
      len ? QSPI_CCR_DMODE_1_DATA_LINES : QSPI_CCR_DMODE_0_DATA_LINES,
      false);
  if (!ok || !qspi_sendCommand(instruction, address, len, data, r_or_w,
                               QSPI_TIMEOUT_PERIOD)) {
    printMsg("QSPI refused command %02X\n\r", instruction);
    return false;
  }
  return true;
}

// Starts at 0xFF so a refused read looks busy and write protected, not fine.
static uint8_t readRegister(uint8_t instruction) {
  uint8_t value = 0xFF;
  command(instruction, false, 0, &value, 1, QSPI_READ);
  return value;
}

/**
 * Polls status register 1 until BUSY clears, failing after maxUs. A real
 * erase or program keeps the chip busy far longer than one poll, so the first
 * poll must see BUSY: if it doesn't, the command never reached the chip.
 */
static bool waitReady(const char *what, uint32_t maxUs, uint32_t *elapsedUs) {
  uint32_t start = DWT->CYCCNT;
  if (!(readRegister(QSPI_READ_REGISTER_ONE) & SR1_BUSY)) {
    printMsg("%s: chip not busy straight after the command\n\r", what);
    return false;
  }
  do {
    *elapsedUs = (DWT->CYCCNT - start) / (core_Hz / 1000000);
    if (*elapsedUs > maxUs) {
      printMsg("%s: still busy after %lu us\n\r", what,
               (unsigned long)*elapsedUs);
      return false;
    }
  } while (readRegister(QSPI_READ_REGISTER_ONE) & SR1_BUSY);
  return true;
}

static bool erase(uint32_t sector, uint32_t *elapsedUs) {
  return command(QSPI_WRITE_ENABLE, false, 0, NULL, 0, QSPI_WRITE) &&
         command(QSPI_SECTOR_ERASE, true, sector * FLASH_SECTOR_SIZE, NULL, 0,
                 QSPI_WRITE) &&
         waitReady("erase", ERASE_MAX_US, elapsedUs);
}

// Programs the page with the contents of expected.
static bool program(uint32_t page, uint32_t *elapsedUs) {
  return command(QSPI_WRITE_ENABLE, false, 0, NULL, 0, QSPI_WRITE) &&
         command(QSPI_PAGE, true, page * FLASH_PAGE_SIZE, expected,
                 FLASH_PAGE_SIZE, QSPI_WRITE) &&
         waitReady("program", PROGRAM_MAX_US, elapsedUs);
}

// Reads the page and compares it with expected.
static bool checkPage(uint32_t page, const char *what) {
  memset(actual, 0, FLASH_PAGE_SIZE);
  if (!command(QSPI_READ_DATA, true, page * FLASH_PAGE_SIZE, actual,
               FLASH_PAGE_SIZE, QSPI_READ)) {
    return false;
  }
  for (uint32_t j = 0; j < FLASH_PAGE_SIZE; j++) {
    if (actual[j] != expected[j]) {
      printMsg("%s: page %lu byte %lu read %02X, expected %02X\n\r", what,
               (unsigned long)page, (unsigned long)j, actual[j], expected[j]);
      return false;
    }
  }
  return true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                              FUNCTION TESTS                               */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

// A floating or shorted line can't produce Winbond's ID by accident.
static bool test_jedecId() {
  uint8_t id[3] = {0};
  command(JEDEC_ID, false, 0, id, 3, QSPI_READ);
  printMsg("jedecId: %02X %02X %02X\n\r", id[0], id[1], id[2]);
  return id[0] == 0xEF && id[1] == 0x40 && id[2] == 0x18;
}

static bool test_statusRegisters() {
  uint8_t sr1 = readRegister(QSPI_READ_REGISTER_ONE);
  uint8_t sr2 = readRegister(QSPI_READ_REGISTER_TWO);
  uint8_t sr3 = readRegister(READ_REGISTER_THREE);
  printMsg("statusRegisters: SR1 %02X SR2 %02X SR3 %02X (QE %d)\n\r", sr1, sr2,
           sr3, (sr2 & QSPI_QUAD_REGISTER) ? 1 : 0);

  // Idle, not suspended, and nothing write protected. CMP set with BP clear
  // would protect the whole array, and WPS switches to per-block locks.
  return (sr1 & (SR1_BUSY | SR1_BP)) == 0 && (sr2 & (SR2_CMP | SR2_SUS)) == 0 &&
         (sr3 & SR3_WPS) == 0;
}

// The latch only changes if the chip decodes the commands we send it.
static bool test_writeEnableLatch() {
  command(QSPI_WRITE_ENABLE, false, 0, NULL, 0, QSPI_WRITE);
  uint8_t set = readRegister(QSPI_READ_REGISTER_ONE);
  command(QSPI_WRITE_DISABLE, false, 0, NULL, 0, QSPI_WRITE);
  uint8_t cleared = readRegister(QSPI_READ_REGISTER_ONE);

  if (!(set & SR1_WEL) || (cleared & SR1_WEL)) {
    printMsg("writeEnableLatch: SR1 %02X after 06, %02X after 04\n\r", set,
             cleared);
    return false;
  }
  return true;
}

static bool test_erase() {
  uint32_t us;
  if (!erase(TEST_SECTOR, &us)) {
    return false;
  }
  printMsg("erase: sector %d took %lu us\n\r", TEST_SECTOR, (unsigned long)us);

  memset(expected, 0xFF, FLASH_PAGE_SIZE);
  for (uint32_t i = 0; i < FLASH_PAGES_PER_SECTOR; i++) {
    if (!checkPage(TEST_PAGE + i, "erase")) {
      return false;
    }
  }
  return true;
}

#define NUM_PATTERNS 5

static void fillPattern(uint32_t pattern) {
  const uint8_t fills[] = {0x00, 0x55, 0xAA};
  for (uint32_t j = 0; j < FLASH_PAGE_SIZE; j++) {
    expected[j] = pattern < 3    ? fills[pattern]
                  : pattern == 3 ? (uint8_t)(1 << (j % 8)) // walking one
                                 : (uint8_t)j;             // counter
  }
}

// Programs every pattern before reading any back, so a write that lands on
// the wrong page also shows up.
static bool test_patterns() {
  uint32_t us;
  uint32_t slowestUs = 0;
  for (uint32_t p = 0; p < NUM_PATTERNS; p++) {
    fillPattern(p);
    if (!program(TEST_PAGE + p, &us)) {
      return false;
    }
    slowestUs = us > slowestUs ? us : slowestUs;
  }
  printMsg("patterns: slowest page program took %lu us\n\r",
           (unsigned long)slowestUs);

  for (uint32_t p = 0; p < NUM_PATTERNS; p++) {
    fillPattern(p);
    if (!checkPage(TEST_PAGE + p, "patterns")) {
      return false;
    }
  }
  return true;
}

// NOR programming can only clear bits, so programming twice without an erase
// leaves the AND of both writes. Anything that just stores the last write, or
// ignores one of them, reads back something else.
static bool test_norOverwrite() {
  uint32_t page = TEST_PAGE + NUM_PATTERNS;
  uint32_t us;

  for (uint32_t j = 0; j < FLASH_PAGE_SIZE; j++) {
    expected[j] = j;
  }
  if (!program(page, &us)) {
    return false;
  }
  memset(expected, 0x5A, FLASH_PAGE_SIZE);
  if (!program(page, &us)) {
    return false;
  }

  for (uint32_t j = 0; j < FLASH_PAGE_SIZE; j++) {
    expected[j] = j & 0x5A;
  }
  return checkPage(page, "norOverwrite");
}

// Page 0 plus pages 1, 2, 4, ... 32768: one page per address bit 8 to 23.
#define NUM_WALK_PAGES 17

static uint32_t walkPage(uint32_t i) {
  return i == 0 ? 0 : 1u << (i - 1);
}

// The walk pages leave different remainders mod 251, so no two get the same
// stamp.
static void fillStamp(uint32_t page) {
  for (uint32_t j = 0; j < FLASH_PAGE_SIZE; j++) {
    expected[j] = (uint8_t)((page + j) % 251);
  }
}

// Stamps every walk page, then reads them all back. A stuck or shorted
// address line sends one page's write or read to another page, which then
// holds the wrong stamp.
static bool test_addressWalk() {
  uint32_t us;

  // Pages 0 to 8 share sector 0; each higher page has its own sector.
  if (!erase(0, &us)) {
    return false;
  }
  for (uint32_t sector = 1; sector <= FLASH_MAX_SECTOR; sector <<= 1) {
    if (!erase(sector, &us)) {
      return false;
    }
  }

  for (uint32_t i = 0; i < NUM_WALK_PAGES; i++) {
    fillStamp(walkPage(i));
    if (!program(walkPage(i), &us)) {
      return false;
    }
  }
  for (uint32_t i = 0; i < NUM_WALK_PAGES; i++) {
    fillStamp(walkPage(i));
    if (!checkPage(walkPage(i), "addressWalk")) {
      return false;
    }
  }
  return true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                            testFunction Core                              */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void testFunction_QSPI() {
  bool (*testFunctions[QSPI_NUM_TESTS])() = {
      test_jedecId,  test_statusRegisters, test_writeEnableLatch,
      test_erase,    test_patterns,        test_norOverwrite,
      test_addressWalk};

  const char *testNames[QSPI_NUM_TESTS] = {
      "jedecId", "statusRegisters", "writeEnableLatch", "erase",
      "patterns", "norOverwrite",   "addressWalk"};

  // The cycle counter times erases and programs.
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  printMsg("\n\rQSPI Tests\n\r\n\r");

  for (uint8_t i = 0; i < QSPI_NUM_TESTS; i++) {
    printMsg("%s: %s\n\r", testNames[i], testFunctions[i]() ? "OK" : "FAIL");
  }
}

/**
 * @file LOGGER_testing.c
 *
 * @brief Tests for virtual_loggers on littlefs. Run as TEST_ID 7.
 *
 * Every run formats the FLASH, erasing anything stored on it. The tests run in
 * order, and each one closes the log file it opens so the next can open one.
 *
 * The run ends by leaving two log files behind: one closed, one still open.
 * Reset the board and the next run checks what survived, before formatting.
 * The file left open is only reported, not failed: logRecord() does not sync,
 * so rows written since the last sync are expected to be lost until a sync
 * policy is chosen.
 *
 * @author Li, Chun Ho (lchli@ucdavis.edu)
 * @date 09/18/2026
 */

#include "virtual_loggers.h"
#include <print_scan.h>
#include <stdlib.h>
#include <string.h>

#define LOGGER_NUM_TESTS 7

#define LOG_NAME "log_test.csv"
#define START_NAME "start_test.csv"
#define JUNK_NAME "not_a_log.txt"
#define CLOSED_NAME "persist_closed.csv"
#define OPEN_NAME "persist_open.csv"

// About 26 rows fit in a 4 KB block, so 60 rows cross two block boundaries.
#define LOG_ROWS 60
#define START_ROWS 3

/**
 * Distinct for every row and field, with alternating signs, so a swapped field
 * or row shows up as a wrong value.
 */
static double fieldValue(uint32_t row, int field) {
  double v = (row * 16 + field + 1) * 1.25e-3;
  return (field % 2) ? -v : v;
}

static int fieldCount(LOG_FILE *f) {
  return f->header == detumbleLOG ? 9 : 7;
}

/**
 * Logs one record filled with fieldValue(row, ...). The struct fields are in
 * the same order logRecord() writes them to the row.
 */
static void logRow(LOG_FILE *f, uint32_t row) {
  double v[9];
  for (int k = 0; k < 9; k++) {
    v[k] = fieldValue(row, k);
  }

  if (f->header == detumbleLOG) {
    detumbleLOGdata d = {
        {v[0], v[1], v[2]}, {v[3], v[4], v[5]}, {v[6], v[7], v[8]}};
    logRecord(f, &d);
  } else {
    detumbleSTARTdata d = {{v[0], v[1], v[2]}, {v[3], v[4], v[5]}, v[6]};
    logRecord(f, &d);
  }
}

/**
 * Reads the next rows records and checks every value against fieldValue().
 * Timestamps are not checked, since vi_get_epoch() is still a stub.
 */
static bool checkRows(LOG_FILE *f, uint32_t rows) {
  for (uint32_t i = 0; i < rows; i++) {
    uint8_t *row = readRecord(f);
    if (!row) {
      printMsg("row %lu: missing\n\r", (unsigned long)i);
      return false;
    }

    // readRecord() does not terminate the row, and strtod() needs it to.
    char text[LOG_ROW_MAX + 1];
    memcpy(text, row, f->row_size);
    text[f->row_size] = '\0';
    if (text[f->row_size - 1] != '\n') {
      printMsg("row %lu: does not end in a newline\n\r", (unsigned long)i);
      return false;
    }

    char *p = text + LOG_TS_WIDTH;
    for (int k = 0; k < fieldCount(f); k++) {
      if (*p != ',') {
        printMsg("row %lu: field %d missing\n\r", (unsigned long)i, k);
        return false;
      }
      double got = strtod(p + 1, &p);
      double want = fieldValue(i, k);
      double diff = got - want;
      if (diff < 0) {
        diff = -diff;
      }
      // "%13.5e" keeps 6 significant digits.
      if (diff > 1e-5 * (want < 0 ? -want : want)) {
        printMsg("row %lu: field %d has the wrong value\n\r",
                 (unsigned long)i, k);
        return false;
      }
    }
  }
  return true;
}

static bool checkHeader(LOG_FILE *f, const char *tag) {
  uint8_t *row = readHeader(f);
  if (!row || strncmp((char *)row, tag, strlen(tag)) != 0 ||
      row[f->row_size - 1] != '\n') {
    printMsg("header: expected a row starting with %s\n\r", tag);
    return false;
  }
  return true;
}

static bool isExitRow(uint8_t *row, const char *message) {
  const char *exit_tag = ", EXIT_LOG_FILE, ";
  size_t tag_len = strlen(exit_tag);
  return row &&
         strncmp((char *)row + LOG_TS_WIDTH, exit_tag, tag_len) == 0 &&
         strncmp((char *)row + LOG_TS_WIDTH + tag_len, message,
                 strlen(message)) == 0;
}

/**
 * Checks for name as a whole line of an lsRecord() listing.
 */
static bool isListed(const char *listing, const char *name) {
  size_t len = strlen(name);
  for (const char *p = listing; (p = strstr(p, name)); p += len) {
    if ((p == listing || p[-1] == '\n') && p[len] == '\n') {
      return true;
    }
  }
  return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                              FUNCTION TESTS                               */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

static bool test_format() {
  int err = lfs_format(&lfs, &cfg);
  if (err) {
    printMsg("format failed (%d)\n\r", err);
    return false;
  }
  err = lfs_mount(&lfs, &cfg);
  if (err) {
    printMsg("mount failed (%d)\n\r", err);
    return false;
  }
  return true;
}

// Only one log file may be open at a time.
static bool test_singleOpen() {
  LOG_FILE *f = createFile(LOG_NAME, detumbleLOG);
  if (!f) {
    printMsg("singleOpen: createFile failed\n\r");
    return false;
  }

  bool ok = true;
  if (createFile(START_NAME, detumbleSTART)) {
    printMsg("singleOpen: a second createFile succeeded\n\r");
    ok = false;
  }
  if (openFile(LOG_NAME)) {
    printMsg("singleOpen: openFile succeeded with a file already open\n\r");
    ok = false;
  }

  closeFile(f, "singleOpen");
  return ok;
}

static bool test_logRecords() {
  LOG_FILE *f = createFile(LOG_NAME, detumbleLOG);
  if (!f) {
    printMsg("logRecords: createFile failed\n\r");
    return false;
  }

  for (uint32_t i = 0; i < LOG_ROWS; i++) {
    logRow(f, i);
  }

  bool ok = checkHeader(f, "detumbleLOG,") && checkRows(f, LOG_ROWS);
  if (ok && readRecord(f)) {
    printMsg("logRecords: more rows than were logged\n\r");
    ok = false;
  }

  // Every row has to be the same width for readRecord() to seek to it.
  lfs_soff_t size = lfs_file_size(&lfs, &f->file);
  if (size != (lfs_soff_t)((LOG_ROWS + 1) * f->row_size)) {
    printMsg("logRecords: file is %ld bytes, expected %lu\n\r", (long)size,
             (unsigned long)((LOG_ROWS + 1) * f->row_size));
    ok = false;
  }

  closeFile(f, "Completed");
  return ok;
}

// Reopening recovers the file's tag from its header, and closeFile() left an
// exit row after the records.
static bool test_reopen() {
  LOG_FILE *f = openFile(LOG_NAME);
  if (!f) {
    printMsg("reopen: openFile failed\n\r");
    return false;
  }

  bool ok = true;
  if (f->header != detumbleLOG) {
    printMsg("reopen: read the wrong tag from the header\n\r");
    ok = false;
  }
  if (ok && !checkRows(f, LOG_ROWS)) {
    ok = false;
  }
  if (ok && !isExitRow(readRecord(f), "Completed")) {
    printMsg("reopen: exit row missing after the records\n\r");
    ok = false;
  }
  if (ok && readRecord(f)) {
    printMsg("reopen: rows after the exit row\n\r");
    ok = false;
  }

  closeFile(f, "reopen");
  return ok;
}

// The same round trip for the other file_tag, which has a narrower row.
static bool test_startFile() {
  LOG_FILE *f = createFile(START_NAME, detumbleSTART);
  if (!f) {
    printMsg("startFile: createFile failed\n\r");
    return false;
  }
  for (uint32_t i = 0; i < START_ROWS; i++) {
    logRow(f, i);
  }
  closeFile(f, "Completed");

  f = openFile(START_NAME);
  if (!f) {
    printMsg("startFile: openFile failed\n\r");
    return false;
  }

  bool ok = true;
  if (f->header != detumbleSTART) {
    printMsg("startFile: read the wrong tag from the header\n\r");
    ok = false;
  }
  if (ok) {
    ok = checkHeader(f, "detumbleSTART,") && checkRows(f, START_ROWS);
  }

  closeFile(f, "startFile");
  return ok;
}

// openFile() refuses a missing file and a file that is not a log, and
// neither refusal leaves the single log file slot taken.
static bool test_openRejects() {
  if (openFile("missing.csv")) {
    printMsg("openRejects: opened a missing file\n\r");
    return false;
  }

  lfs_file_t junk;
  const char text[] = "this is not a log file\n";
  int err = lfs_file_opencfg(&lfs, &junk, JUNK_NAME,
                             LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC,
                             &fs_file_cfg);
  if (!err) {
    lfs_file_write(&lfs, &junk, text, sizeof(text) - 1);
    err = lfs_file_close(&lfs, &junk);
  }
  if (err) {
    printMsg("openRejects: could not write %s (%d)\n\r", JUNK_NAME, err);
    return false;
  }

  if (openFile(JUNK_NAME)) {
    printMsg("openRejects: opened a file that is not a log\n\r");
    return false;
  }

  LOG_FILE *f = openFile(LOG_NAME);
  if (!f) {
    printMsg("openRejects: a refused open left the slot taken\n\r");
    return false;
  }
  closeFile(f, "openRejects");
  return true;
}

// The FLASH was formatted at the start, so exactly the files written so far
// should be listed.
static bool test_lsRecord() {
  char *listing = lsRecord();

  int lines = 0;
  for (char *p = listing; *p; p++) {
    lines += *p == '\n';
  }

  if (lines != 3 || !isListed(listing, LOG_NAME) ||
      !isListed(listing, START_NAME) || !isListed(listing, JUNK_NAME)) {
    printMsg("lsRecord: expected %s, %s and %s, got %d files\n\r", LOG_NAME,
             START_NAME, JUNK_NAME, lines);
    return false;
  }
  return true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                              PERSISTENCE                                  */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

/**
 * Checks the files the previous run left behind. Needs a mounted filesystem.
 */
static void checkPersistence() {
  LOG_FILE *f = openFile(CLOSED_NAME);
  if (!f) {
    printMsg("persistence: no files from a previous run, skipped\n\r");
    return;
  }
  bool ok = checkRows(f, LOG_ROWS) && isExitRow(readRecord(f), "Completed") &&
            !readRecord(f);
  closeFile(f, "checked");
  printMsg("persistence (closed file): %s\n\r", ok ? "OK" : "FAIL");

  f = openFile(OPEN_NAME);
  if (!f) {
    printMsg("persistence (left open): file missing\n\r");
    return;
  }
  uint32_t rows = 0;
  while (readRecord(f)) {
    rows++;
  }
  closeFile(f, "checked");
  printMsg("persistence (left open): %lu of %u rows survived\n\r",
           (unsigned long)rows, LOG_ROWS);
}

/**
 * Leaves one closed and one open log file for the next run to check.
 */
static void writePersistence() {
  LOG_FILE *f = createFile(CLOSED_NAME, detumbleLOG);
  if (!f) {
    printMsg("\n\rCould not create %s\n\r", CLOSED_NAME);
    return;
  }
  for (uint32_t i = 0; i < LOG_ROWS; i++) {
    logRow(f, i);
  }
  closeFile(f, "Completed");

  f = createFile(OPEN_NAME, detumbleLOG);
  if (!f) {
    printMsg("\n\rCould not create %s\n\r", OPEN_NAME);
    return;
  }
  for (uint32_t i = 0; i < LOG_ROWS; i++) {
    logRow(f, i);
  }
  // Left open on purpose, as if the board reset mid-log.

  printMsg("\n\rPersistence files written. Reset the board to check them.\n\r");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                            testFunction Core                              */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void testFunction_LOGGER() {
  bool (*testFunctions[LOGGER_NUM_TESTS])() = {
      test_format,    test_singleOpen,  test_logRecords, test_reopen,
      test_startFile, test_openRejects, test_lsRecord};

  const char *testNames[LOGGER_NUM_TESTS] = {
      "format",    "singleOpen",  "logRecords", "reopen",
      "startFile", "openRejects", "lsRecord"};

  printMsg("\n\rLOGGER Tests\n\r\n\r");

  // init_platform() already ran fs_mount(), but threw away its result. Mount
  // again here to get a result we can check. Unmounting first is safe: with
  // every buffer static, it only resets littlefs's state.
  lfs_unmount(&lfs);
  if (lfs_mount(&lfs, &cfg) == 0) {
    checkPersistence();
  } else {
    printMsg("persistence: mount failed, skipped\n\r");
  }
  lfs_unmount(&lfs);

  for (uint8_t i = 0; i < LOGGER_NUM_TESTS; i++) {
    printMsg("%s: %s\n\r", testNames[i], testFunctions[i]() ? "OK" : "FAIL");
  }

  writePersistence();
}

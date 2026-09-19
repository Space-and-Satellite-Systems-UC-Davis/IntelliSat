/**
 * @file FS_testing.c
 *
 * @brief Bring-up tests for littlefs on the W25Q128JV FLASH. Run as TEST_ID 6.
 *
 * Every run formats the FLASH, erasing anything stored on it. The tests run in
 * order and build on each other: later tests read the files earlier ones wrote.
 *
 * The run ends by writing a marker file. Reset the board and the next run
 * checks that the marker survived, before formatting again.
 *
 * @author Li, Chun Ho (lchli@ucdavis.edu)
 * @date 09/18/2026
 */

#include "lfs_config.h"
#include <print_scan.h>
#include <string.h>

#define FS_NUM_TESTS 8

#define SMALL_FILE "fs_test_small"
#define LARGE_FILE "fs_test_large"
#define PERSIST_FILE "fs_test_persist"

// Small enough for littlefs to store inline in its metadata, with no data block.
#define SMALL_SIZE 100
// Spans several blocks, so reads and writes cross block boundaries.
#define LARGE_SIZE (3 * FS_BLOCK_SIZE + 123)
#define APPEND_SIZE 1000
#define PERSIST_SIZE (FS_BLOCK_SIZE + 500)

// Neither divides the 256 byte page, so chunks straddle page boundaries.
#define WRITE_CHUNK 100
#define READ_CHUNK 77

#define CREATE_FLAGS (LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC)

// fs_file_cfg holds a single cache buffer, so only one file may be open at a
// time.
static lfs_file_t file;

// 251 is prime, so the pattern never repeats in step with a page or block, and
// a page read from the wrong place shows up as wrong bytes.
static uint8_t patternByte(uint32_t offset) {
  return (uint8_t)(offset % 251);
}

/**
 * Writes pattern bytes [from, to) to a file opened with the given flags.
 */
static bool writePattern(const char *name, uint32_t from, uint32_t to,
                         int flags) {
  uint8_t chunk[WRITE_CHUNK];

  int err = lfs_file_opencfg(&lfs, &file, name, flags, &fs_file_cfg);
  if (err) {
    printMsg("%s: open failed (%d)\n\r", name, err);
    return false;
  }

  bool ok = true;
  for (uint32_t done = from; ok && done < to; done += WRITE_CHUNK) {
    uint32_t n = lfs_min(WRITE_CHUNK, to - done);
    for (uint32_t j = 0; j < n; j++) {
      chunk[j] = patternByte(done + j);
    }
    lfs_ssize_t written = lfs_file_write(&lfs, &file, chunk, n);
    if (written != (lfs_ssize_t)n) {
      printMsg("%s: write failed at offset %lu (%ld)\n\r", name,
               (unsigned long)done, (long)written);
      ok = false;
    }
  }

  // Closing is what commits the data, so it has to succeed too.
  err = lfs_file_close(&lfs, &file);
  if (err) {
    printMsg("%s: close failed (%d)\n\r", name, err);
    return false;
  }
  return ok;
}

/**
 * Checks that a file is exactly size bytes long and holds the pattern.
 */
static bool checkPattern(const char *name, uint32_t size) {
  uint8_t chunk[READ_CHUNK];

  int err = lfs_file_opencfg(&lfs, &file, name, LFS_O_RDONLY, &fs_file_cfg);
  if (err) {
    printMsg("%s: open failed (%d)\n\r", name, err);
    return false;
  }

  bool ok = true;
  lfs_soff_t actual = lfs_file_size(&lfs, &file);
  if (actual != (lfs_soff_t)size) {
    printMsg("%s: size is %ld, expected %lu\n\r", name, (long)actual,
             (unsigned long)size);
    ok = false;
  }

  for (uint32_t done = 0; ok && done < size;) {
    lfs_ssize_t n = lfs_file_read(&lfs, &file, chunk, READ_CHUNK);
    if (n <= 0) {
      printMsg("%s: read failed at offset %lu (%ld)\n\r", name,
               (unsigned long)done, (long)n);
      ok = false;
      break;
    }
    for (lfs_ssize_t j = 0; j < n; j++) {
      if (chunk[j] != patternByte(done + j)) {
        printMsg("%s: wrong byte at offset %lu\n\r", name,
                 (unsigned long)(done + j));
        ok = false;
        break;
      }
    }
    done += n;
  }

  lfs_file_close(&lfs, &file);
  return ok;
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

static bool test_smallFile() {
  return writePattern(SMALL_FILE, 0, SMALL_SIZE, CREATE_FLAGS) &&
         checkPattern(SMALL_FILE, SMALL_SIZE);
}

static bool test_largeFile() {
  return writePattern(LARGE_FILE, 0, LARGE_SIZE, CREATE_FLAGS) &&
         checkPattern(LARGE_FILE, LARGE_SIZE);
}

static bool test_seek() {
  // On either side of the page and block sizes.
  const lfs_soff_t offsets[] = {0,    255,  256,  4095,
                                4096, 4097, 8192, LARGE_SIZE - 1};
  uint8_t byte;

  int err = lfs_file_opencfg(&lfs, &file, LARGE_FILE, LFS_O_RDONLY,
                             &fs_file_cfg);
  if (err) {
    printMsg("seek: open failed (%d)\n\r", err);
    return false;
  }

  bool ok = true;
  for (size_t i = 0; ok && i < sizeof(offsets) / sizeof(offsets[0]); i++) {
    lfs_soff_t off = offsets[i];
    if (lfs_file_seek(&lfs, &file, off, LFS_SEEK_SET) != off ||
        lfs_file_read(&lfs, &file, &byte, 1) != 1 ||
        byte != patternByte(off)) {
      printMsg("seek: wrong byte at offset %ld\n\r", (long)off);
      ok = false;
    }
  }

  lfs_file_close(&lfs, &file);
  return ok;
}

// virtual_loggers opens its files with LFS_O_APPEND, so test that path too.
static bool test_append() {
  return writePattern(LARGE_FILE, LARGE_SIZE, LARGE_SIZE + APPEND_SIZE,
                      LFS_O_WRONLY | LFS_O_APPEND) &&
         checkPattern(LARGE_FILE, LARGE_SIZE + APPEND_SIZE);
}

// Remounting drops littlefs's caches, so the reads have to come from the FLASH.
static bool test_remount() {
  int err = lfs_unmount(&lfs);
  if (!err) {
    err = lfs_mount(&lfs, &cfg);
  }
  if (err) {
    printMsg("remount failed (%d)\n\r", err);
    return false;
  }
  return checkPattern(LARGE_FILE, LARGE_SIZE + APPEND_SIZE);
}

// The FLASH was formatted at the start, so exactly the two files written so
// far should be listed.
static bool test_dirList() {
  lfs_dir_t dir;
  struct lfs_info info;

  int err = lfs_dir_open(&lfs, &dir, "/");
  if (err) {
    printMsg("dirList: open failed (%d)\n\r", err);
    return false;
  }

  int files = 0;
  bool small = false;
  bool large = false;
  while (lfs_dir_read(&lfs, &dir, &info) > 0) {
    if (info.type != LFS_TYPE_REG) {
      continue; // skips "." and ".."
    }
    files++;
    small |= strcmp(info.name, SMALL_FILE) == 0;
    large |= strcmp(info.name, LARGE_FILE) == 0;
  }
  lfs_dir_close(&lfs, &dir);

  if (files != 2 || !small || !large) {
    printMsg("dirList: found %d files, expected %s and %s\n\r", files,
             SMALL_FILE, LARGE_FILE);
    return false;
  }
  return true;
}

static bool test_remove() {
  struct lfs_info info;
  lfs_ssize_t before = lfs_fs_size(&lfs);

  int err = lfs_remove(&lfs, LARGE_FILE);
  if (err) {
    printMsg("remove failed (%d)\n\r", err);
    return false;
  }
  if (lfs_stat(&lfs, LARGE_FILE, &info) != LFS_ERR_NOENT) {
    printMsg("remove: file still exists\n\r");
    return false;
  }

  // The removed file's blocks should go back to the free pool.
  lfs_ssize_t after = lfs_fs_size(&lfs);
  if (after < 0 || after >= before) {
    printMsg("remove: %ld blocks in use before, %ld after\n\r", (long)before,
             (long)after);
    return false;
  }
  return true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                            testFunction Core                              */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void testFunction_FS() {
  bool (*testFunctions[FS_NUM_TESTS])() = {
      test_format, test_smallFile, test_largeFile, test_seek,
      test_append, test_remount,   test_dirList,   test_remove};

  const char *testNames[FS_NUM_TESTS] = {
      "format", "smallFile", "largeFile", "seek",
      "append", "remount",   "dirList",   "remove"};

  printMsg("\n\rFS Tests\n\r\n\r");

  // init_platform() already ran fs_mount(), but threw away its result. Mount
  // again here to get a result we can check. Unmounting first is safe: with
  // every buffer static, it only resets littlefs's state.
  struct lfs_info info;
  lfs_unmount(&lfs);
  if (lfs_mount(&lfs, &cfg) == 0 && lfs_stat(&lfs, PERSIST_FILE, &info) == 0) {
    printMsg("persistence: %s\n\r",
             checkPattern(PERSIST_FILE, PERSIST_SIZE) ? "OK" : "FAIL");
  } else {
    printMsg("persistence: no marker from a previous run, skipped\n\r");
  }
  lfs_unmount(&lfs);

  for (uint8_t i = 0; i < FS_NUM_TESTS; i++) {
    printMsg("%s: %s\n\r", testNames[i], testFunctions[i]() ? "OK" : "FAIL");
  }

  if (writePattern(PERSIST_FILE, 0, PERSIST_SIZE, CREATE_FLAGS)) {
    printMsg("\n\rMarker written. Reset the board to check persistence.\n\r");
  }
}

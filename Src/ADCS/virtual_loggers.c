/**
 * @file virtual_loggers.c
 *
 * @brief littlefs-backed implementation of ADCS Software's logger interface.
 *
 * Log files are fixed-width CSV. Row 0 is the column header and every row
 * after it is one record, so readRecord() can seek to a row directly.
 *
 * @author Li, Chun Ho (lchli@ucdavis.edu)
 * @date 09/11/2026
 */

#include "virtual_loggers.h"
#include "virtual_intellisat.h"

#include <stdbool.h>
#include <string.h>

// littlefs is built with LFS_NO_MALLOC and fs_file_cfg holds a single cache
// buffer, so exactly one log file may be open at a time.
static LOG_FILE log_file;
static bool log_file_open = false;

static char row_buf[LOG_ROW_MAX];    // scratch for the row being formatted
static uint8_t read_buf[LOG_ROW_MAX];// returned by readHeader / readRecord
static char ls_buf[1024];            // returned by lsRecord

// Indexed by file_tag. LOG_TS_WIDTH, then ", " + a number per field, then '\n'.
static const size_t row_sizes[] = {
    [detumbleLOG] = LOG_TS_WIDTH + 9 * (2 + LOG_NUM_WIDTH) + 1,
    [detumbleSTART] = LOG_TS_WIDTH + 7 * (2 + LOG_NUM_WIDTH) + 1,
};

#define TS_FMT "%04d-%02d-%02d_%02d-%02d-%02d"
#define NUM_FMT ", %13.5e"

/**
 * Pads whatever snprintf left in row_buf out to row_size, terminates it with a
 * newline and writes it. Keeping every write behind this is what holds the
 * uniform-row invariant that readRecord() depends on.
 */
static bool writeRow(LOG_FILE *file) {
  size_t len = strlen(row_buf);
  if (len > file->row_size - 1) {
    len = file->row_size - 1;
  }
  memset(row_buf + len, ' ', file->row_size - 1 - len);
  row_buf[file->row_size - 1] = '\n';

  lfs_ssize_t written =
      lfs_file_write(&lfs, &file->file, row_buf, file->row_size);
  return written == (lfs_ssize_t)file->row_size;
}

static void stampNow(TIMESTAMP *ts) {
  if (vi_get_epoch(&ts->year, &ts->month, &ts->day, &ts->hour, &ts->minute,
                   &ts->second) != GET_EPOCH_SUCCESS) {
    memset(ts, 0, sizeof(*ts));
  }
}

LOG_FILE *createFile(char *name, file_tag tag) {
  if (log_file_open) {
    return NULL;
  }

  int err = lfs_file_opencfg(&lfs, &log_file.file, name,
                             LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC |
                                 LFS_O_APPEND,
                             &fs_file_cfg);
  if (err) {
    return NULL;
  }

  log_file.row = 0;
  log_file.row_size = row_sizes[tag];
  log_file.header = tag;
  log_file_open = true;

  logHeader(&log_file);
  // Commit the header so the file survives a reset even if nothing is logged.
  lfs_file_sync(&lfs, &log_file.file);

  return &log_file;
}

LOG_FILE *openFile(char *name) {
  if (log_file_open) {
    return NULL;
  }

  int err = lfs_file_opencfg(&lfs, &log_file.file, name,
                             LFS_O_RDWR | LFS_O_APPEND, &fs_file_cfg);
  if (err) {
    return NULL;
  }

  // The tag is not stored anywhere but the header row, so recover it from the
  // name that logHeader() wrote there.
  char tag_buf[14];
  lfs_ssize_t read = lfs_file_read(&lfs, &log_file.file, tag_buf, sizeof(tag_buf));
  if (read == (lfs_ssize_t)sizeof(tag_buf) &&
      strncmp(tag_buf, "detumbleSTART,", 14) == 0) {
    log_file.header = detumbleSTART;
  } else if (read == (lfs_ssize_t)sizeof(tag_buf) &&
             strncmp(tag_buf, "detumbleLOG,", 12) == 0) {
    log_file.header = detumbleLOG;
  } else {
    lfs_file_close(&lfs, &log_file.file);
    return NULL;
  }

  log_file.row = 0;
  log_file.row_size = row_sizes[log_file.header];
  log_file_open = true;

  return &log_file;
}

void closeFile(LOG_FILE *file, char *message) {
  TIMESTAMP ts;
  stampNow(&ts);

  snprintf(row_buf, LOG_ROW_MAX, TS_FMT ", EXIT_LOG_FILE, %s", ts.year,
           ts.month, ts.day, ts.hour, ts.minute, ts.second, message);
  writeRow(file);

  lfs_file_close(&lfs, &file->file);
  log_file_open = false;
}

void logHeader(LOG_FILE *file) {
  switch (file->header) {
  case detumbleLOG:
    snprintf(row_buf, LOG_ROW_MAX,
             "detumbleLOG, mag_x, mag_y, mag_z, imu_x, imu_y, imu_z, mdm_x, "
             "mdm_y, mdm_z");
    break;
  case detumbleSTART:
    snprintf(row_buf, LOG_ROW_MAX,
             "detumbleSTART, imu_x, imu_y, imu_z, mag_x, mag_y, mag_z, temp");
    break;
  }
  writeRow(file);
}

void logRecord(LOG_FILE *file, void *record) {
  TIMESTAMP ts;
  stampNow(&ts);

  switch (file->header) {
  case detumbleLOG: {
    detumbleLOGdata *d = (detumbleLOGdata *)record;
    snprintf(row_buf, LOG_ROW_MAX,
             TS_FMT NUM_FMT NUM_FMT NUM_FMT NUM_FMT NUM_FMT NUM_FMT NUM_FMT
                 NUM_FMT NUM_FMT,
             ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, d->mag.x,
             d->mag.y, d->mag.z, d->imu.x, d->imu.y, d->imu.z, d->mdm.x,
             d->mdm.y, d->mdm.z);
    break;
  }
  case detumbleSTART: {
    detumbleSTARTdata *d = (detumbleSTARTdata *)record;
    snprintf(row_buf, LOG_ROW_MAX,
             TS_FMT NUM_FMT NUM_FMT NUM_FMT NUM_FMT NUM_FMT NUM_FMT NUM_FMT,
             ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, d->imu.x,
             d->imu.y, d->imu.z, d->mag.x, d->mag.y, d->mag.z, d->temp);
    break;
  }
  }

  // No sync here. Syncing per record would erase a FLASH page per record; the
  // cache is flushed when it fills and on closeFile().
  writeRow(file);
}

uint8_t *readHeader(LOG_FILE *file) {
  if (lfs_file_seek(&lfs, &file->file, 0, LFS_SEEK_SET) < 0) {
    return NULL;
  }
  if (lfs_file_read(&lfs, &file->file, read_buf, file->row_size) !=
      (lfs_ssize_t)file->row_size) {
    return NULL;
  }
  return read_buf;
}

uint8_t *readRecord(LOG_FILE *file) {
  lfs_soff_t off = (lfs_soff_t)((file->row + 1) * file->row_size);
  if (lfs_file_seek(&lfs, &file->file, off, LFS_SEEK_SET) < 0) {
    return NULL;
  }
  if (lfs_file_read(&lfs, &file->file, read_buf, file->row_size) !=
      (lfs_ssize_t)file->row_size) {
    return NULL;
  }
  file->row++;
  return read_buf;
}

char *lsRecord() {
  // lfs_info alone carries a 256 byte name, so neither of these goes on the
  // stack.
  static lfs_dir_t dir;
  static struct lfs_info info;

  ls_buf[0] = '\0';
  if (lfs_dir_open(&lfs, &dir, "/") < 0) {
    return ls_buf;
  }

  size_t used = 0;
  while (lfs_dir_read(&lfs, &dir, &info) > 0) {
    if (info.type != LFS_TYPE_REG) {
      continue; // skips "." and ".."
    }
    size_t need = strlen(info.name) + 1;
    if (used + need + 1 > sizeof(ls_buf)) {
      break;
    }
    used += snprintf(ls_buf + used, sizeof(ls_buf) - used, "%s\n", info.name);
  }

  lfs_dir_close(&lfs, &dir);
  return ls_buf;
}

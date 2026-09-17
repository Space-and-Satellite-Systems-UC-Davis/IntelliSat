/**
 * @file lfs_config.c
 *
 * @brief Block device callbacks mapping littlefs onto the W25Q128JV QSPI FLASH.
 *
 * @author Li, Chun Ho (lchli@ucdavis.edu)
 * @date 09/09/2026
 */

#include "lfs_config.h"
#include "../peripherals/FLASH/W25Q128JV.h"

lfs_t lfs;

// Static buffers, 32-bit aligned. LFS_NO_MALLOC is set in lfs_util.h, so
// littlefs will not allocate these itself.
static uint32_t global_read_buf[FS_CACHE_SIZE / 4];
static uint32_t global_prog_buf[FS_CACHE_SIZE / 4];
static uint32_t global_look_buf[FS_LOOKAHEAD_SIZE / 4];
static uint32_t global_file_buf[FS_CACHE_SIZE / 4];

const struct lfs_file_config fs_file_cfg = {
    .buffer = global_file_buf,
};

// littlefs addresses the device as (block, byte offset); the driver addresses
// it by page number. read_size and prog_size are both one page, so every
// offset and length littlefs hands us is already page aligned.
static uint32_t fs_toPage(lfs_block_t block, lfs_off_t off) {
  return (block * FS_BLOCK_SIZE + off) / FLASH_PAGE_SIZE;
}

static int fs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                   void *buffer, lfs_size_t size) {
  (void)c;
  if (!flash_readCustom(fs_toPage(block, off), (uint8_t *)buffer, size)) {
    return LFS_ERR_IO;
  }
  return 0;
}

static int fs_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                   const void *buffer, lfs_size_t size) {
  (void)c;
  if (!flash_writeCustom(fs_toPage(block, off), (uint8_t *)buffer, size)) {
    return LFS_ERR_IO;
  }
  return 0;
}

static int fs_erase(const struct lfs_config *c, lfs_block_t block) {
  (void)c;
  // One littlefs block is one FLASH sector, so the indices line up.
  if (!flash_eraseSector(block)) {
    return LFS_ERR_IO;
  }
  return 0;
}

static int fs_sync(const struct lfs_config *c) {
  (void)c;
  // Every driver call already blocks on flash_wait() before returning, so
  // nothing is left buffered in the device.
  return 0;
}

const struct lfs_config cfg = {
    // block device operations
    .read = fs_read,
    .prog = fs_prog,
    .erase = fs_erase,
    .sync = fs_sync,

    // block device configuration
    .read_size = FS_READ_SIZE,
    .prog_size = FS_PROG_SIZE,
    .block_size = FS_BLOCK_SIZE,
    .block_count = FS_BLOCK_COUNT,
    .cache_size = FS_CACHE_SIZE,
    .lookahead_size = FS_LOOKAHEAD_SIZE,
    .block_cycles = 500,

    // statically allocated buffers
    .read_buffer = global_read_buf,
    .prog_buffer = global_prog_buf,
    .lookahead_buffer = global_look_buf,
};

int fs_mount() {
  int err = lfs_mount(&lfs, &cfg);
  if (err) {
    // No valid filesystem on the part yet (or it is corrupt) - lay one down.
    err = lfs_format(&lfs, &cfg);
    if (err) {
      return err;
    }
    err = lfs_mount(&lfs, &cfg);
  }
  return err;
}

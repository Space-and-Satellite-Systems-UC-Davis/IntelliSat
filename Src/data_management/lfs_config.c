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

/**
 * @brief Convert a littlefs (block, offset) location into a FLASH page number
 *
 * @param block littlefs block index (one block is one FLASH sector)
 * @param off byte offset within the block
 *
 * @note littlefs addresses the device as (block, byte offset); the driver
 *       addresses it by page number. read_size and prog_size are both one
 *       page, so every offset littlefs hands us is already page aligned and
 *       the division never drops a remainder.
 *
 * @return page number to pass to the FLASH driver
 */
static uint32_t fs_toPage(lfs_block_t block, lfs_off_t off) {
  return (block * FS_BLOCK_SIZE + off) / FLASH_PAGE_SIZE;
}

/**
 * @brief littlefs read callback: read bytes out of a block
 *
 * @param c littlefs config (unused, there is only one device)
 * @param block block to read from
 * @param off byte offset within the block, a multiple of FS_READ_SIZE
 * @param buffer destination for the data
 * @param size number of bytes to read, a multiple of FS_READ_SIZE
 *
 * @return 0 on success, or LFS_ERR_IO if the driver reports a failure
 */
static int fs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                   void *buffer, lfs_size_t size) {
  (void)c;
  if (!flash_readCustom(fs_toPage(block, off), (uint8_t *)buffer, size)) {
    return LFS_ERR_IO;
  }
  return 0;
}

/**
 * @brief littlefs prog callback: program bytes into a block
 *
 * @param c littlefs config (unused, there is only one device)
 * @param block block to program
 * @param off byte offset within the block, a multiple of FS_PROG_SIZE
 * @param buffer data to write
 * @param size number of bytes to write, a multiple of FS_PROG_SIZE
 *
 * @note littlefs only programs regions it has already erased with fs_erase(),
 *       so this never needs to erase first.
 *
 * @return 0 on success, or LFS_ERR_IO if the driver reports a failure
 */
static int fs_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                   const void *buffer, lfs_size_t size) {
  (void)c;
  // The driver takes a non-const buffer but only reads from it.
  if (!flash_writeCustom(fs_toPage(block, off), (uint8_t *)buffer, size)) {
    return LFS_ERR_IO;
  }
  return 0;
}

/**
 * @brief littlefs erase callback: erase a whole block
 *
 * @param c littlefs config (unused, there is only one device)
 * @param block block to erase
 *
 * @return 0 on success, or LFS_ERR_IO if the driver reports a failure
 */
static int fs_erase(const struct lfs_config *c, lfs_block_t block) {
  (void)c;
  // One littlefs block is one FLASH sector, so the indices line up.
  if (!flash_eraseSector(block)) {
    return LFS_ERR_IO;
  }
  return 0;
}

/**
 * @brief littlefs sync callback: flush any data the device still holds
 *
 * @param c littlefs config (unused, there is only one device)
 *
 * @return always 0
 */
static int fs_sync(const struct lfs_config *c) {
  (void)c;
  // Every driver call already blocks on flash_wait() before returning, so
  // nothing is left buffered in the device.
  return 0;
}

// Handed to lfs_mount() / lfs_format(). Geometry comes from lfs_config.h.
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
    // Erase cycles before littlefs moves metadata to a new block, spreading
    // wear across the part.
    .block_cycles = 500,

    // statically allocated buffers
    .read_buffer = global_read_buf,
    .prog_buffer = global_prog_buf,
    .lookahead_buffer = global_look_buf,
};

/**
 * @brief Mount the filesystem, formatting the FLASH first if needed
 *
 * @note Call once after qspi_config(). If the FLASH does not hold a valid
 *       littlefs image (first boot, or it is corrupt) it is formatted, which
 *       erases anything already on it.
 *
 * @return 0 on success, or a negative LFS_ERR_* code
 */
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

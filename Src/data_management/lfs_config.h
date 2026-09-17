/**
 * @file lfs_config.h
 *
 * @brief littlefs bound to the W25Q128JV QSPI FLASH.
 *
 * @author Li, Chun Ho (lchli@ucdavis.edu)
 * @date 09/09/2026
 */

#ifndef DATA_MANAGEMENT_LFS_CONFIG_H_
#define DATA_MANAGEMENT_LFS_CONFIG_H_

#include "../peripherals/FLASH/W25Q128JV.h"
#include "littlefs/lfs.h"

// Filesystem geometry. The block size is the FLASH's smallest erase unit
// (one 4 KB sector), since flash_eraseSector() is the only erase the driver
// exposes. 4096 sectors * 4096 bytes = the full 16 MB part.
#define FS_READ_SIZE FLASH_PAGE_SIZE
#define FS_PROG_SIZE FLASH_PAGE_SIZE
#define FS_BLOCK_SIZE FLASH_SECTOR_SIZE
#define FS_BLOCK_COUNT (FLASH_MAX_SECTOR + 1)
#define FS_CACHE_SIZE FLASH_PAGE_SIZE
#define FS_LOOKAHEAD_SIZE 32

extern lfs_t lfs;
extern const struct lfs_config cfg;

// Buffer for a single open file. littlefs is built with LFS_NO_MALLOC, so
// every lfs_file_opencfg() needs one of these, sized to FS_CACHE_SIZE.
extern const struct lfs_file_config fs_file_cfg;

/**
 * Mounts the filesystem, formatting the FLASH first if it does not yet hold a
 * valid littlefs image. Call once after qspi_config().
 *
 * @returns 0 on success, or a negative LFS_ERR_* code.
 */
int fs_mount(void);

#endif /* DATA_MANAGEMENT_LFS_CONFIG_H_ */

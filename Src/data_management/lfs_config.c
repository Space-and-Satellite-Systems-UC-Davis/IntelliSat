#include "Src/data_management/littlefs/lfs.h"

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

// configuration of the filesystem is provided by this struct

// TODO: Set up static global buffers so we can avoid using malloc
#define SECTOR_CACHE_SIZE 256 // Must match your read/program block size
#define LOOKAHEAD_SIZE 32     // Must be a multiple of 8

// Create 32-bit aligned static memory arrays
static uint32_t global_read_buf[SECTOR_CACHE_SIZE / 4];
static uint32_t global_prog_buf[SECTOR_CACHE_SIZE / 4];
static uint32_t global_look_buf[LOOKAHEAD_SIZE / 4];

const struct lfs_config cfg = {
    // block device operations
    .read = user_provided_block_device_read,
    .prog = user_provided_block_device_prog,
    .erase = user_provided_block_device_erase,
    .sync = user_provided_block_device_sync,

    // Pass your static arrays directly to the config
    .read_buffer = global_read_buf,
    .prog_buffer = global_prog_buf,
    .lookahead_buffer = global_look_buf,

    // block device configuration
    .read_size = 16,
    .prog_size = 16,
    .block_size = 65536,
    .block_count = 256,
    .cache_size = 16,
    .lookahead_size = 16,
    .block_cycles = 500,
};

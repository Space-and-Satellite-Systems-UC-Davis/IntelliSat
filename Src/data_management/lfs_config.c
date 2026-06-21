#include "Src/data_management/littlefs/lfs.h"

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

// configuration of the filesystem is provided by this struct

// TODO: Set up static global buffers so we can avoid using malloc

const struct lfs_config cfg = {
    // block device operations
    .read = user_provided_block_device_read,
    .prog = user_provided_block_device_prog,
    .erase = user_provided_block_device_erase,
    .sync = user_provided_block_device_sync,

    // block device configuration
    .read_size = 16,
    .prog_size = 16,
    .block_size = 65536,
    .block_count = 256,
    .cache_size = 16,
    .lookahead_size = 16,
    .block_cycles = 500,
};

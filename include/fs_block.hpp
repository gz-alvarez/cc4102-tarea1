#ifndef FS_BLOCK_HPP
#define FS_BLOCK_HPP

#include <cstddef>
#include <print>
#include <sys/statvfs.h>


extern size_t B_bytes;


size_t fetch_fs_block_size();


#endif

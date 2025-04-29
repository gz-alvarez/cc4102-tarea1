#include <fs_block.hpp>


size_t B_bytes(fetch_fs_block_size());


size_t fetch_fs_block_size() {
	struct statvfs buf;
	statvfs(".", &buf);
	std::println("Detected block size: {}B", buf.f_bsize);
	return buf.f_bsize;
}

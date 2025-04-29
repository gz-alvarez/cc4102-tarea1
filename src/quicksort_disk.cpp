#include "quicksort_disk.hpp"

#include <algorithm>
#include <fstream>


void quicksort_disk(DiskArray<uint64_t> &bin) {
	// placeholder: regular quicksort for testing
	// load the entire file into a big array and just sort it normally, then write it again
	std::vector<uint64_t> arr;

	// read
	for (size_t block_idx=0; block_idx<bin.size(); block_idx++) {
		std::vector<uint64_t> block = bin[block_idx];
		arr.insert(arr.begin(), block.begin(), block.end());
	}

	std::sort(arr.begin(), arr.end()); // sort in memory

	// write
	for (size_t block_idx=0; block_idx<bin.size(); block_idx++) {
		std::vector<uint64_t> block(arr.begin()+block_idx*B_bytes/sizeof(uint64_t), arr.end()+block_idx*B_bytes/sizeof(uint64_t));
		bin[block_idx] = block;
	}

}
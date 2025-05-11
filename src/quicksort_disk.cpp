#include "quicksort_disk.hpp"

#include <constrained_memory.hpp>

#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <set>  
#include <iostream>


void quicksort_disk(DiskArrayQ<uint64_t> &bin) {
	size_t N_bytes = bin.size() * B_bytes;
	if (N_bytes <= M_bytes) {
		//se ordena el arreglo en mem principal
		std::vector<uint64_t> arr;

		for (size_t block_idx=0; block_idx<bin.size(); block_idx++) {
			std::vector<uint64_t> block = bin[block_idx];
			arr.insert(arr.begin(), block.begin(), block.end());
		}

		std::sort(arr.begin(), arr.end()); 

		for (size_t block_idx=0; block_idx<bin.size(); block_idx++) {
			size_t start = block_idx * B_bytes / sizeof(uint64_t);
			size_t end = std::min(start + B_bytes / sizeof(uint64_t), arr.size());
			std::vector<uint64_t> block(arr.begin() + start, arr.begin() + end);
			bin[block_idx] = block;
		}
	}
	else {
		//leer bloque aleatorio
		size_t a = 62; 
		std::vector<uint64_t> pivots;
		std::set<uint64_t> ids;

		size_t total_elements = bin.size() * (B_bytes / sizeof(uint64_t));
		size_t n_pivots = a - 1;
		while (pivots.size() < n_pivots) {
			size_t block_id = std::rand() % bin.size();
			std::vector<uint64_t> block = bin[block_id];
			size_t idx = std::rand() % block.size();
			uint64_t val = block[idx];
			if (ids.insert(val).second) {
				pivots.push_back(val);
			}
		}
		// ordena los pivotes
		std::sort(pivots.begin(), pivots.end());

		// particionar en subarreglos
		std::vector<std::string> t_files(a);
		std::vector<std::ofstream> t_streams(a);

		for (size_t i = 0; i < a; i++) {
			t_files[i] = "t_" + std::to_string(i) + ".bin";
			t_streams[i].open(t_files[i], std::ios::binary);
		}

		for (size_t block_id = 0; block_id < bin.size(); block_id++) {
			std::vector<uint64_t> block = bin[block_id];
			for (uint64_t val : block) {
				size_t part_id = 0;
				while (part_id < pivots.size() && val > pivots[part_id]) {
					part_id++;
				}
				t_streams[part_id].write(reinterpret_cast<char*>(&val), sizeof(uint64_t));
			}
		}

		for (auto &stream : t_streams) stream.close();

		//llamado recursivo a quicksort
		std::vector<DiskArrayQ<uint64_t>> sorted_arrays;
		for (const auto &file : t_files) {
			DiskArrayQ<uint64_t> subarray(file, B_bytes / sizeof(uint64_t));
			quicksort_disk(subarray);
			sorted_arrays.push_back(std::move(subarray));
		}

		//concatenar los subarreglos
		size_t block_id = 0;
		size_t pos = 0;
		std::vector<uint64_t> vec_block(B_bytes / sizeof(uint64_t));

		for (const auto &subarray : sorted_arrays) {
			for (size_t i = 0; i < subarray.size(); i++) {
				std::vector<uint64_t> block = subarray[i];
				for (uint64_t val : block) {
					if (pos == vec_block.size()) {
						bin[block_id] = vec_block;
						block_id++;
						vec_block.resize(B_bytes / sizeof(uint64_t));
						pos = 0;
					}
					vec_block[pos] = val;
					pos++;
				}
			}
		}
		if (pos > 0) {
			bin[block_id] = vec_block;
		}
	}
}
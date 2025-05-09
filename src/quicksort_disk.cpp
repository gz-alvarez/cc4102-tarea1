#include "quicksort_disk.hpp"

#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <set>  
#include <iostream>


const size_t M = 50 * 1024 * 1024;
void quicksort_disk(DiskArray<uint64_t> &bin) {
	size_t N = bin.size() * B_bytes / sizeof(uint64_t);
	if (N <= M) {
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
		size_t block_id_random = std::rand() % bin.size();
		std::vector<uint64_t> block_random = bin[block_id_random];

		size_t a = 10; //falta definir a (se necesita haber implementado mergesort externo)
		size_t block_size = block_random.size();
		std::vector<uint64_t> pivots;
		std::set<size_t> ids;

		for(size_t i = 0; i < a - 1; i++){
			size_t random_id;
			//evita que se repita el id al elegir al azar
			do {
				random_id = std::rand() % block_size;
			} while (ids.find(random_id) != ids.end());
			ids.insert(random_id);
			pivots.push_back(block_random[random_id]);
		}
	
		// ordena los pivotes
		std::sort(pivots.begin(), pivots.end());

		// particionar en subarreglos
		std::vector<std::vector<uint64_t>> subarrays(a);

		for (size_t i = 0; i < block_size; i++){
			uint64_t val = block_random[i];
			size_t part_id = 0;
			while(part_id < pivots.size() && val > pivots[part_id]){
				part_id++;
			}
			subarrays[part_id].push_back(val);
		}

		//llamado recursivo a quicksort

		std::vector<std::string> files(subarrays.size());
		std::vector<DiskArray<uint64_t>> sorted_array;
		std::vector<int> nums;

		for (size_t i = 0; i < subarrays.size(); i++){
			files[i] = "file_" + std::to_string(i) + ".bin";
			std::ofstream out(files[i], std::ios::binary);
			out.write(reinterpret_cast<char*>(subarrays[i].data()), subarrays[i].size() * sizeof(uint64_t));
			out.close();
			sorted_array.push_back(DiskArray<uint64_t>(files[i], block_size)); 
			quicksort_disk(sorted_array[i]);
		}

		//concatenar los subarreglos
		size_t block_id = 0;
		size_t pos = 0;
		std::vector<uint64_t> vec_block = bin[block_id];

		for (size_t i = 0; i < sorted_array.size(); i++) {
			DiskArray<uint64_t>& sorted_subarray = sorted_array[i];
			for (size_t j = 0; j < sorted_subarray.size(); j++) {	
				std::vector<uint64_t> block = sorted_subarray[j];
				for (size_t k = 0; k < block.size(); k++) {
					if (pos < vec_block.size()) {
						vec_block[pos] = block[k];
						pos++;
						} else {
							bin[block_id] = vec_block;        
							block_id++;
							pos = 0;
							vec_block = bin[block_id];     
							vec_block[pos] = block[k];
							pos++;
						}
				}
			}
		}
		bin[block_id] = vec_block; 
	}
}
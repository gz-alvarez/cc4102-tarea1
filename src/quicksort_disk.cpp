#include "quicksort_disk.hpp"

#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <set>  


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
			std::vector<uint64_t> block(arr.begin()+block_idx*B_bytes/sizeof(uint64_t), arr.end()+block_idx*B_bytes/sizeof(uint64_t));
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
			pivots.push_back(block_random[random_id])
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
		for (size_t i = 0; i < subarrays.size(); i++){
			DiskArray<uint64_t> subarray_disk(subarrays[i]);
			quicksort_disk(subarray_disk);
		}

		//concatenar los subarreglos
		size_t block_id = 0;
		size_t pos = 0;
		
		for (size_t subarray_idx = 0; subarray_idx < subarrays.size(); subarray_idx++) {
			std::vector<uint64_t>& subarray = subarrays[subarray_idx];
			for (size_t subarray_element_idx = 0; subarray_element_idx < subarray.size(); subarray_element_idx++) {
				uint64_t val = subarray[subarray_element_idx];
				bin[block_id][pos] = val;
				pos++;
				if(pos == block_size){
					block_id++;
					pos = 0;
				}
			}
		}
	}
}
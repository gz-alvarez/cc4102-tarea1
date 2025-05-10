#include "mergesort_disk.hpp"

#include <constrained_memory.hpp>

#include <algorithm>
#include <limits>


// retorna el numero de lecturas y escrituras realizadas en archivos temporales distintos de bin
std::pair<unsigned, unsigned> mergesort_disk_rec(DiskArray<uint64_t> &bin, size_t start_idx, size_t end_idx, unsigned int arity) {
	size_t block_count = end_idx - start_idx;
	if (block_count * B_bytes < M_bytes) {
		// si los bloques caben en memoria, podemos cargar todo y ordenar
		std::vector<uint64_t> mbuffer(block_count * B_bytes / sizeof(uint64_t));
		for (size_t i=0; i<block_count; i++) {
			// cargar cada bloque en la parte correspondiente de mbuffer
			std::vector<uint64_t> block = bin[start_idx + i];
			std::copy(block.begin(), block.end(), mbuffer.begin() + i * B_bytes / sizeof(uint64_t));
		}

		std::sort(mbuffer.begin(), mbuffer.end()); // ordenar

		// escribir los bloques de vuelta
		for (size_t i=0; i<block_count; i++) {
			std::vector<uint64_t> block(
				mbuffer.begin() + i * B_bytes / sizeof(uint64_t),
				mbuffer.begin() + (i+1) * B_bytes / sizeof(uint64_t)
			);
			bin[start_idx + i] = block;
		}

		return std::make_pair(0,0); // no se hizo IO a archivos temporales
	} else {
		// si no cabe todo, dividimos en (aridad) llamadas recursivas y recombinamos
		std::pair<unsigned, unsigned> total_tmp_ios = std::make_pair(0,0);
		std::vector<size_t> start(arity);
		std::vector<size_t> end(arity);
		for (size_t i=0; i<arity; i++) {
			// definir los segmentos de cada llamada recursiva
			start[i] = start_idx + (i * block_count) / arity;
			end[i] = start_idx + ((i + 1) * block_count) / arity;
			auto tmp_ios = mergesort_disk_rec(bin, start[i], end[i], arity); // llamada recursiva: quedarán ordenados sus segmentos correspondientes
			total_tmp_ios.first += tmp_ios.first;
			total_tmp_ios.second += tmp_ios.second;
		}

		DiskArray<uint64_t> merge("merge.tmp.bin", block_count, true); // para la combinación se crea un nuevo archivo temporal
		std::vector<uint64_t> mbuffer(B_bytes / sizeof(uint64_t)); // buffer de un bloque, que se escribirá al archivo cada vez que se llene
		size_t merge_current_block = 0;
		size_t mbuffer_current_idx = 0;

		std::vector<std::vector<uint64_t>> segm_loaded_block(arity); // mantenemos en memoria (aridad) bloques, uno para cada segmento que está siendo leido
		std::vector<size_t> segm_block_count(arity, 0); // tamaño en bloques de cada segmento
		std::vector<size_t> segm_current_block(arity, 0); // indices que indican el progreso en cada segmento
		std::vector<size_t> segm_current_elem(arity, 0); // indices que indican el progreso dentro del bloque actual de cada segmento
		for (size_t i=0; i<arity; i++) {
			segm_block_count[i] = end[i] - start[i];
			segm_loaded_block[i] = bin[start[i]]; // IO. cargar el primer bloque de cada segmento
		}

		size_t elem_count = block_count * B_bytes / sizeof(uint64_t); // cantidad total de numeros
		for (size_t elem_idx=0; elem_idx<elem_count; elem_idx++) {
			uint64_t min(std::numeric_limits<uint64_t>::max()); // iteraremos constantemente sobre los segmentos para encontrar el minimo
			size_t min_segm;
			for (size_t segm_i=0; segm_i<arity; segm_i++) {
				// iterar sobre cada segmento en su bloque cargado y consultar su elemento actual
				// para encontrar cual de todos los segmentos tiene el minimo
				bool segm_merge_complete = segm_current_block[segm_i] >= segm_block_count[segm_i]; // chequear si es que ya se leyó todo el segmento
				uint64_t segm_elem = segm_loaded_block[segm_i][segm_current_elem[segm_i]];
				if (!segm_merge_complete && segm_elem < min) {
					min = segm_elem; // asignar minimo
					min_segm = segm_i;
				}
			}
			// terminado lo anterior, tenemos el siguiente elemento minimo
			// avanzar el segmento del que vino (porque lo estamos "extraendo")
			segm_current_elem[min_segm]++;
			if (segm_current_elem[min_segm] >= segm_loaded_block[min_segm].size()) {
				// avanzar al siguiente bloque si es necesario
				segm_current_block[min_segm]++;
				// solo si aun quedan bloques por leer de este segmento, cargar el siguiente
				if (segm_current_block[min_segm] < segm_block_count[min_segm]) {
					segm_loaded_block[min_segm] = bin[start[min_segm] + segm_current_block[min_segm]]; // IO
				} 
				segm_current_elem[min_segm] = 0; // volver a al inicio del bloque
			}

			// insertar el minimo al buffer
			mbuffer[mbuffer_current_idx++] = min;
			if (mbuffer_current_idx >= mbuffer.size()) {
				// si el buffer está lleno, escribirlo al archivo temporal y volver al inicio del buffer
				merge[merge_current_block] = mbuffer; // IO
				merge_current_block++;
				mbuffer_current_idx = 0;
			}
		}

		// ya se combinaron todos los segmentos al archivo temporal, todos sus elementos estan ordenados
		// ahora copiar el contenido del archivo temporal al original en el lugar que corresponde
		// el archivo merge se destruye automaticamente
		for (size_t cp_i=0; cp_i<block_count; cp_i++) {
			std::vector<uint64_t> block = merge[cp_i]; // IO
			bin[start_idx + cp_i] = block; // IO
		}

		return std::make_pair(total_tmp_ios.first + merge.reads(), total_tmp_ios.second + merge.writes());
	}

};

std::pair<unsigned, unsigned> mergesort_disk(DiskArray<uint64_t> &bin, unsigned int arity) {
	return mergesort_disk_rec(bin, 0, bin.size(), arity);
}
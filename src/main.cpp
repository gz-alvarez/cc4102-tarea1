#include <array>
#include <algorithm>
#include <random>
#include <filesystem>
#include <thread>
#include <chrono>
#include <print>
#include <functional>
#include <iostream>
#include <DiskArrayQ.hpp>
#include <DiskArray.hpp>
#include <fs_block.hpp>
#include <quicksort_disk.hpp>
#include <mergesort_disk.hpp>
#include <constrained_memory.hpp>


std::random_device rd;
using time_unit = std::chrono::microseconds;


struct ExperimentResult {
	size_t blocks;
	std::random_device::result_type seed;
	time_unit qs_time;
	time_unit ms_time;
	unsigned int qs_reads;
	unsigned int ms_reads;
	unsigned int qs_writes;
	unsigned int ms_writes;
	explicit operator std::string() const {
		return std::format("TIME QS:{} MS:{} | READS QS:{} MS:{} | WRITES QS:{} MS:{}", qs_time, ms_time, qs_reads, ms_reads, qs_writes, ms_writes);
	}
};


ExperimentResult experiment(int N_megabytes, unsigned int arity) {
	// Primero, vemos la cantidad de bloques necesitados para N_megabytes: ceil(n)
	// La cantidad de elementos a utilizar corresponderá a ceil(n) en vez de solo n,
	// es decir, solo trabajaremos con bloques completos para simplificar la implementación
	uint64_t N_bytes = (uint64_t)N_megabytes * 1'000'000;
	size_t size_blocks = ceil((float)N_bytes / B_bytes); // ceil(n)
	std::println("Experiment for N = {}MB => n = {} blocks, for a total of {}MB", N_megabytes, size_blocks, size_blocks*B_bytes/1'000'000.0);

	// Iniciar generador para este conjunto de números con semilla aleatoria
	auto seed = rd();
	std::mt19937_64 rng(seed);
	
	// Crear los archivo binarios en los que se trabajará
	DiskArray<uint64_t> qs_bin("sequenceQS.bin", size_blocks);
	DiskArray<uint64_t> ms_bin("sequenceMS.bin", size_blocks);

	// Insertar los n bloques
	for (size_t block_idx=0; block_idx<size_blocks ; block_idx++) {
		// Crear arreglo en memoria de 1 block y llenarlo con números aleatorios
		std::vector<uint64_t> buffer(B_bytes / sizeof(uint64_t));
		std::generate(buffer.begin(), buffer.end(), std::ref(rng));

		// escribir el arreglo a los archivos
		qs_bin[block_idx] = buffer;
		ms_bin[block_idx] = buffer;
	}

	// Aqui llamar a la implementación de QS/MS y obtener datos
	unsigned int qs_reads_old = qs_bin.reads();
	unsigned int qs_writes_old = qs_bin.writes();
	auto qs_begin = std::chrono::steady_clock::now();
	quicksort_disk(qs_bin);
	auto qs_time = std::chrono::duration_cast<time_unit>(std::chrono::steady_clock::now() - qs_begin);
	unsigned int qs_reads = qs_bin.reads() - qs_reads_old;
	unsigned int qs_writes = qs_bin.writes() - qs_writes_old;

	unsigned int ms_reads_old = ms_bin.reads();
	unsigned int ms_writes_old = ms_bin.writes();
	auto ms_begin = std::chrono::steady_clock::now();
	mergesort_disk(ms_bin, arity);
	auto ms_time = std::chrono::duration_cast<time_unit>(std::chrono::steady_clock::now() - ms_begin);
	unsigned int ms_reads = ms_bin.reads() - ms_reads_old;
	unsigned int ms_writes = ms_bin.writes() - ms_writes_old;

	// Eliminar archivos
	std::filesystem::remove("sequenceQS.bin");
	std::filesystem::remove("sequenceMS.bin");

	ExperimentResult res{
		.blocks = size_blocks,
		.seed = seed,
		.qs_time = qs_time,
		.ms_time = ms_time,
		.qs_reads = qs_reads,
		.ms_reads = ms_reads,
		.qs_writes = qs_writes,
		.ms_writes = ms_writes
	};
	std::println("{}", std::string(res));

	return res;
}


unsigned int compute_a() {
	size_t size_blocks = ceil(60.0 * M_megabytes * 1'000'000.0 / B_bytes); // 60M megabytes -> bloques
	size_t b = B_bytes / sizeof(uint64_t);
	std::pair<size_t, size_t> abounds(2, b);
	while (abounds.second - abounds.first >= 2) {
		std::println("Bounds: {}", abounds);
		auto seed = rd();
		std::mt19937_64 rng(seed);
		DiskArray<uint64_t> mbin1("msort_arity_1.tmp.bin", size_blocks, true);
		DiskArray<uint64_t> mbin2("msort_arity_2.tmp.bin", size_blocks, true);
		for (size_t block_i=0; block_i<size_blocks; block_i++) {
			// llenar archivos al azar
			std::vector<uint64_t> buffer(B_bytes / sizeof(uint64_t));
			std::generate(buffer.begin(), buffer.end(), std::ref(rng));
			// escribir el arreglo a los archivos
			mbin1[block_i] = buffer;
			mbin2[block_i] = buffer;
		}
		size_t m1 = abounds.first + (abounds.second - abounds.first) / 3;
		size_t m2 = abounds.first + (abounds.second - abounds.first) * (2.0 / 3.0);
		// ordenar y comparar IOs
		auto tmp_ios_1 = mergesort_disk(mbin1, m1);
		std::println("m1: {}, tmp_ios_1: {}", m1, tmp_ios_1);
		auto tmp_ios_2 = mergesort_disk(mbin2, m2);
		std::println("m2: {}, tmp_ios_2: {}", m2, tmp_ios_2);
		unsigned int ios_total_1 = tmp_ios_1.first + tmp_ios_1.second + mbin1.reads() + mbin1.writes();
		unsigned int ios_total_2 = tmp_ios_2.first + tmp_ios_2.second + mbin2.reads() + mbin2.writes();
		if (ios_total_1 < ios_total_2) {
			abounds = std::make_pair(abounds.first, m2);
		} else {
			abounds = std::make_pair(m1, abounds.second);
		}
	}

	return (abounds.second - abounds.first) / 2;
}


int main(int, char**){
	/*
	Se deberá generar 5 secuencias de números enteros de 64 bits de tamaño total 𝑁 , con
	𝑁 ∈ {4𝑀, 8𝑀, …60𝑀} (es decir, 5 secuencias de tamaño 4𝑀, 5 secuencias de tamaño 8𝑀, …), inser-
	tarlos desordenadamente en un arreglo y guardarlo en binario en disco. Por cada uno de esos arreglos en
	binario, se tendrá que realizar lo siguiente
	*/

	std::println("Using M[MB] = {}", M_megabytes);
	constexpr int REPEAT_COUNT = 5;

	constexpr int N_count = 60/4;
	std::array<int, N_count> N_list_megabytes; // lista de Ns = {4M, 8M, ...}
	for (int i=0; i<=N_count; i++) {
		N_list_megabytes[i] = 4 * (i+1) * M_megabytes;
	}
	std::println("Using N[MB] = {}", N_list_megabytes);

	std::println("Computing arity...");
	unsigned int arity = compute_a();
	std::println("Decided on a = {}", arity);

	std::println("Starting experiments");
	for (auto N_megabytes : N_list_megabytes) {
		for (int repeat=0; repeat<REPEAT_COUNT; repeat++) {
			auto result = experiment(N_megabytes, arity);
		}
	}
}
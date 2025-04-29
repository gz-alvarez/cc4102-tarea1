#ifndef DISKARRAY_HPP
#define DISKARRAY_HPP

#include <fstream>
#include <print>
#include <vector>

#include <fs_block.hpp>


/*
Clase que representa un archivo en disco de tamaño n bloques, donde cada bloque i en [0,n) puede ser leído o escrito.
*/
template <typename T>
class DiskArray {
	std::fstream stream;
	const size_t size_blocks;
	unsigned int stat_read = 0;
	unsigned int stat_write = 0;

	class Block {
		int index;
		DiskArray& da;

		public:
		Block(DiskArray& bf, int index) : da(bf), index(index) {}

		// Write
		void operator=(const std::vector<T> buf){
			std::streampos oldp = da.stream.tellp();
			da.stream.seekp(index * B_bytes);
			da.stream.write(reinterpret_cast<const char*>(buf.data()), B_bytes);
			da.stream.seekp(oldp);
			da.stat_write++;
		}

		// Read
		operator std::vector<T>() const {
			std::vector<T> res(B_bytes/sizeof(T), 0);
			std::streampos oldp = da.stream.tellp();
			da.stream.seekp(index * B_bytes);
			da.stream.read(reinterpret_cast<char*>(res.data()), B_bytes);
			da.stream.seekp(oldp);
			da.stat_read++;
			return res;
		}
	};

	public:
	DiskArray(std::filesystem::path path, size_t size_blocks): size_blocks(size_blocks) {
		stream.open(path, std::ios::out | std::ios::in | std::ios::trunc);
		if (not stream.is_open()) throw std::runtime_error(std::format("Failed to open file {}", path.string()));
	}
	Block operator[](int index) {
		if (index >= size_blocks) {
			throw std::out_of_range(std::format("Not a valid index: {}", index));
		}
		return Block(*this, index);
	}
	unsigned int reads() const {return stat_read;}
	unsigned int writes() const {return stat_write;}
	// Size of the array in blocks
	size_t size() const {return size_blocks;}
};


#endif

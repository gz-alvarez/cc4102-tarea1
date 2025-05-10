#ifndef MERGESORT_DISK_HPP
#define MERGESORT_DISK_HPP

#include <cstdint>
#include <utility>

#include <DiskArray.hpp>


std::pair<unsigned, unsigned> mergesort_disk(DiskArray<uint64_t>& bin, unsigned int arity);


#endif

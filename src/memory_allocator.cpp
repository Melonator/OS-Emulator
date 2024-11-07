#include "../include/memory_allocator.h"

#include <algorithm>
#include <iostream>
#include <mutex>

using namespace allocator;


FirstFit::FirstFit(size_t size, size_t blockSize) : maximumSize(size) {
    this->blockSize = blockSize;
    memory.reserve(maximumSize);
    initializeMemory();
}

FirstFit::~FirstFit() {
    memory.clear();
}

void *FirstFit::allocate(size_t size, const std::string& name) {
    size_t requiredBlocks = (size + blockSize - 1) / blockSize;
    size_t totalBlocks = maximumSize / blockSize;
    for (size_t i = 0; i <= totalBlocks - requiredBlocks; ++i) {
        if (isBlockFree(i, requiredBlocks)) {
            // Add a new allocation record directly to allocations
            std::lock_guard<std::mutex> lock(allocatorMutex);
            allocations.push_back({i, requiredBlocks, name});
            allocatedSize += size;
            return &memory[i * blockSize];
        }
    }

    return nullptr;
}

void FirstFit::deallocate(void *ptr) {
    size_t startBlock = (static_cast<char*>(ptr) - &memory[0]) / blockSize;

    // Find the corresponding allocation record in allocations
    std::lock_guard<std::mutex> lock(allocatorMutex);
    auto it = std::find_if(allocations.begin(), allocations.end(),
        [startBlock](const AllocationRecord& record) {
            return record.startBlock == startBlock;
        });

    if (it != allocations.end()) {
        allocatedSize -= it->size * blockSize;
        // Remove the allocation record from allocations
        allocations.erase(it);
    }
    // size_t index = static_cast<char*>(ptr) - &memory[0];
    // if (allocations[index])
    //     deallocateAt(index);
}

std::string FirstFit::visualizeMemory() {
    const time_t timestamp = time(NULL);
    struct tm datetime = *localtime(&timestamp);
    char output[50] = "";
    strftime(output, 50, "%m/%d/%Y, %I:%M:%S %p", &datetime);
    const std::string timestampStr = output;
    std::string result = "";
    // current timestamp
    result += "timestamp = " + timestampStr + "\n";
    // number of processes in memory
    // total number of fragmentation in KB
    result += "total number of fragmentation in KB = " + std::to_string(0) + "\n";
    result += "----end---- = " + maximumSize;

    for (size_t i = 0; i < allocations.size(); ++i) {
        // result +=
        for (size_t j = 0; j < allocations[i].size; ++j) {
            std::cout << memory[allocations[i].startBlock + j];
        }
        std::cout << std::endl;
    }
    result += "----start---- = " + 0;
    // return std::string(memory.begin(), memory.end());
    return "";
    // return result;
}

void FirstFit::initializeMemory() {
    std::fill(memory.begin(), memory.end(), '.'); // '.' represents unallocated memory
    // std::fill(allocations.begin(), allocations.end(), false);
}

bool FirstFit::canAllocateAt(size_t index, size_t size) const {
    return (index + size <= maximumSize);
}

bool FirstFit::isBlockFree(size_t startBlock, size_t numBlocks) const {
    for (size_t i = startBlock; i < startBlock + numBlocks; ++i) {
        // Check if any existing allocation overlaps with this block range
        if (std::any_of(allocations.begin(), allocations.end(),
                        [i](const AllocationRecord& record) {
                            return i >= record.startBlock && i < record.startBlock + record.size;
                        })) {
            return false; // Block is not free
                        }
    }
    return true; // Block is free
}

// void FirstFit::allocateAt(size_t index, size_t size) {
//     for (size_t i = index; i < index + size; ++i) {
//         allocations[i] = true;
//     }
//     allocatedSize += size;
//     // std::fill(allocationMap.begin() + index, allocationMap.begin() + index + size, true);
// }

// void FirstFit::deallocateAt(size_t index) {
//     allocations[index] = false;
// }



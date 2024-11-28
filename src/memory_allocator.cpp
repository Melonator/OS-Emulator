#include "../include/memory_allocator.h"

#include <algorithm>
#include <format>
#include <iostream>
#include <mutex>

using namespace allocator;

#pragma region FirstFit
FlatModel::FlatModel(size_t size, size_t blockSize) : maximumSize(size) {
    this->blockSize = blockSize;
    memory.reserve(maximumSize);
    this->allocatedSize = 0;
    initializeMemory();
}

FlatModel::~FlatModel() {
    memory.clear();
}

void *FlatModel::allocate(size_t size, const std::string& name, size_t entranceCycle) {
    size_t requiredBlocks = (size + blockSize - 1) / blockSize;
    size_t totalBlocks = maximumSize / blockSize;
    for (size_t i = 0; i <= totalBlocks - requiredBlocks; ++i) {
        if (isBlockFree(i, requiredBlocks)) {
            // Add a new allocation record directly to allocations
            // std::lock_guard<std::mutex> lock(allocatedMutex);
            allocations.push_back({i, requiredBlocks, name, entranceCycle});
            allocatedSize += size;
            return &memory[i * blockSize];
        }
    }

    return nullptr;
}

void FlatModel::deallocate(void *ptr) {
    size_t startBlock = (static_cast<char*>(ptr) - &memory[0]) / blockSize;

    // Find the corresponding allocation record in allocations
    // std::lock_guard<std::mutex> lock(allocatedMutex);
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

std::string FlatModel::visualizeMemory() {
    // std::lock_guard<std::mutex> lock(allocatedMutex);
    // const time_t timestamp = time(NULL);
    // struct tm datetime = *localtime(&timestamp);
    // char output[50] = "";
    // strftime(output, 50, "%m/%d/%Y, %I:%M:%S %p", &datetime);
    // const std::string timestampStr = output;
    std::string result = "";
    // // current timestamp
    // result += "Timestamp: " + timestampStr + "\n";
    // // number of processes in memory
    // result += "Number of processes in memory: " + std::to_string(allocations.size()) + "\n";
    // // total number of fragmentation in KB
    // result += "total number of fragmentation in KB: " + std::to_string(maximumSize - allocatedSize) + "\n";
    // result += "\n----end---- = " + std::to_string(maximumSize - 1) + "\n\n";
    //
    // for (size_t i = maximumSize - 1;i --> 0 ;) {
    //
    //     for (size_t j = 0; j < allocations.size(); ++j) {
    //         if (allocations[j].startBlock * blockSize == i) {
    //             result +=  (std::to_string((allocations[j].startBlock + allocations[j].size) * blockSize - 1)) + "\n";
    //             result += allocations[j].name + "\n";
    //             result += (std::to_string(allocations[j].startBlock * blockSize)) + "\n\n";
    //         }
    //     }
    //     // std:: cout << std::endl;
    // }
    // result += "\n----start---- = 0\n";
    result += std::to_string(maximumSize) + " K total memory\n";
    result += std::to_string(allocatedSize) + " K used memory\n";
    result += std::to_string(maximumSize - allocatedSize) + " K free memory\n.";
    return result;
}

void FlatModel::initializeMemory() {
    std::fill(memory.begin(), memory.end(), '.'); // '.' represents unallocated memory
    // std::fill(allocations.begin(), allocations.end(), false);
}

bool FlatModel::canAllocateAt(size_t index, size_t size) const {
    return (index + size <= maximumSize);
}

bool FlatModel::isBlockFree(size_t startBlock, size_t numBlocks) const {
    // checks if block from startBlock to startBlock + numBlocks is free
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

void FlatModel::moveToBackingStore(const std::string& name) {
    // std::lock_guard<std::mutex> lock(allocatedMutex);

    for (auto it = allocations.begin(); it != allocations.end(); ++it) {
        if (it->name == name) {
            // std::lock_guard<std::mutex> backingLock(backingStoreMutex);
            // Add allocation record to the backing store
            backingStore.push_back(*it);

            // Calculate the memory pointer for deallocation
            void* ptr = &memory[it->startBlock * blockSize];

            // Use deallocate to free the memory
            deallocate(ptr);

            return; // Exit after moving the desired allocation
        }
    }
}

void *FlatModel::getFromBackingStore(const std::string& name, size_t entranceCycle) {
    // std::lock_guard<std::mutex> lock(backingStoreMutex);
    for (size_t i = 0; i < backingStore.size(); ++i) {
        AllocationRecord record = backingStore[i];

        if (record.name == name) {
            // Try to allocate memory for this record
            void* allocatedMemory = allocate(record.size * blockSize, name, entranceCycle);

            if (allocatedMemory != nullptr) {
                // Allocation successful, update the record with new entranceCycle
                record.entranceCycle = entranceCycle;

                // Add the record to the allocations vector
                allocations.push_back(record);

                // Remove from the backing store
                backingStore.erase(backingStore.begin() + i);

                return allocatedMemory; // Successfully moved back
            }
        }
    }
    return nullptr;
}

std::string FlatModel::getOldestProcessNotRunning(std::vector<std::string> running) {
    // std::lock_guard<std::mutex> allocatedLock(allocatedMutex);
    std::string oldest = "";
    size_t minCycle = -1;
    for (size_t i = 0; i < allocations.size(); i++) {
        if (std::find(running.begin(), running.end(), allocations[i].name) == running.end() && allocations[i].entranceCycle < minCycle) {
        // if (allocations[i].entranceCycle < minCycle) {
            minCycle = allocations[i].entranceCycle;
            oldest = allocations[i].name;
        }
    }
    return oldest;
}

bool FlatModel::inBackingStore(std::string name) {
    for (size_t i = 0; i < backingStore.size(); i++) {
        if (backingStore[i].name == name) {
            return true;
        }
    }
    return false;
}

std::string FlatModel::getUtil() {
    std::string util = "";
    util += "Memory Usage: " + std::to_string(allocatedSize) + "KB / " + std::to_string(maximumSize) + "KB\n";
    util += std::format("Memory Util: {0:.2f}%\n\n", ((float)allocatedSize / maximumSize) * 100);
    return util;
}
#pragma endregion FirstFit

#pragma region Paging
Paging::Paging(size_t size, size_t pageSize) : maximumSize(size), pageSize(pageSize) {
    this->blockSize = 0;
    memory.reserve(maximumSize);
    this->allocatedSize = 0;
    initializeMemory();
}

Paging::~Paging() {
    memory.clear();
}

void Paging::initializeMemory() {
    std::fill(memory.begin(), memory.end(), '.');
    size_t numPages = maximumSize / pageSize;
    for (size_t i = 0; i < numPages; i++) {
        freePages.push_back({i, "", 0});
    }
}

void *Paging::allocate(size_t size, const std::string &name, size_t entranceCycle) {
    size_t reqPages = (size + pageSize - 1) / pageSize;
    std::lock_guard<std::mutex> freeLock(freeMutex);
    std::lock_guard<std::mutex> allocatedLock(allocatedMutex);
    while (reqPages != 0) {
        Page page = freePages.front();
        freePages.erase(freePages.begin());
        page.name = name;
        page.entranceCycle = entranceCycle;
        allocatedPages.push_back(page);
        allocatedSize += pageSize;
        totalPagedIn++;
        reqPages--;
    }
    return nullptr;
}

bool Paging::canAllocate(size_t size) {
    std::lock_guard<std::mutex> lock(freeMutex);
    size_t reqPages = (size + pageSize - 1) / pageSize;
    return freePages.size() >= reqPages;
}


void Paging::deallocate(const std::string& name) {
    std::lock_guard<std::mutex> allocatedLock(allocatedMutex);
    for (size_t i = 0; i < allocatedPages.size(); i++) {
        Page page = allocatedPages[i];
        if (page.name == name) {
            {
                std::lock_guard<std::mutex> freeLock(freeMutex);
                freePages.push_back({page.index, "", 0});
            }
            // page.index = 0;
            allocatedPages.erase(allocatedPages.begin() + i);
            allocatedSize -= pageSize;
            i--;
            totalPagedOut++;
        }
    }
}

std::string Paging::visualizeMemory() {
    std::string result = "";
    result += std::to_string(maximumSize) + " K total memory\n";
    result += std::to_string(allocatedSize) + " K used memory\n";
    result += std::to_string(maximumSize - allocatedSize) + " K free memory\n.";
    result += std::to_string(totalPagedIn) + " pages paged in\n";
    result += std::to_string(totalPagedOut) + " pages paged out\n";
    return result;
}

void Paging::moveToBackingStore(const std::string& name) {
    std::lock_guard<std::mutex> backingLock(backingStoreMutex);
    for (size_t i = 0; i < allocatedPages.size(); i++) {
        Page page = allocatedPages[i];
        if (page.name == name) {
            {
                std::lock_guard<std::mutex> freeLock(freeMutex);
                freePages.push_back({page.index, "", 0});
            }
            page.index = 0;
            backingStore.push_back(page);
            allocatedPages.erase(allocatedPages.begin() + i);
            totalPagedOut++;
            allocatedSize -= pageSize;
            i--;
        }
    }
}

void Paging::getFromBackingStore(const std::string& name, size_t entranceCycle) {
    std::lock_guard<std::mutex> backingLock(backingStoreMutex);
    for (size_t i = 0; i < backingStore.size(); i++) {
        Page page = backingStore[i];
        if (page.name == name) {
            std::lock_guard<std::mutex> freeLock(freeMutex);
            Page freePage = freePages.front();
            freePages.erase(freePages.begin());
            freePage.name = name;
            freePage.entranceCycle = entranceCycle;
            allocatedPages.push_back(freePage);
            backingStore.erase(backingStore.begin() + i);
            totalPagedIn++;
        }
    }
}

bool Paging::inBackingStore(std::string name) {
    std::lock_guard<std::mutex> backingLock(backingStoreMutex);
    for (size_t i = 0; i < backingStore.size(); i++) {
        if (backingStore[i].name == name) {
            return true;
        }
    }
    return false;
}

bool Paging::isAllocated(std::string name) {
    std::lock_guard<std::mutex> allocatedLock(allocatedMutex);
    for (size_t i = 0; i < allocatedPages.size(); i++) {
        if (allocatedPages[i].name == name) {
            return true;
        }
    }
    return false;
}

std::string Paging::getOldestProcessNotRunning(std::vector<std::string> running) {
    std::lock_guard<std::mutex> allocatedLock(allocatedMutex);
    std::string oldest = "";
    size_t minCycle = -1;
    for (size_t i = 0; i < allocatedPages.size(); i++) {
        if (std::find(running.begin(), running.end(), allocatedPages[i].name) == running.end() && allocatedPages[i].entranceCycle < minCycle) {
        // if (allocatedPages[i].entranceCycle < minCycle) {
            minCycle = allocatedPages[i].entranceCycle;
            oldest = allocatedPages[i].name;
        }
    }
    return oldest;
}

size_t Paging::getTotalIn() {
    return totalPagedIn;
}

size_t Paging::getTotalOut() {
    return totalPagedOut;
}

std::string Paging::getUtil() {
    std::string util = "";
    util += "Memory Usage: " + std::to_string(allocatedSize) + "KB / " + std::to_string(maximumSize) + "KB\n";
    util += std::format("Memory Util: {0:.2f}%\n\n", ((float)allocatedSize / maximumSize) * 100);
    return util;
}
#pragma endregion Paging






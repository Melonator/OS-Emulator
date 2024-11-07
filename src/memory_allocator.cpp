#include "../include/memory_allocator.h"

#include <iostream>

using namespace allocator;

FirstFit::FirstFit(size_t size) : maximumSize(size){
    memory.reserve(maximumSize);
    initializeMemory();
}

FirstFit::~FirstFit() {
    memory.clear();
}

void *FirstFit::allocate(size_t size) override {
    for (size_t i; i < maximumSize - size + 1; ++i) {
        if (!allocationMap[i] && canAllocateAt(i, size)) {
            allocateAt(i, size);
            return &memory[i];
        }
    }

    return nullptr;
}

void FirstFit::deallocate(void *ptr) override {
    size_t index = static_cast<char*>(ptr) - &memory[0];
    if (allocationMap[index])
        deallocateAt(index);
}

std::string FirstFit::visualizeMemory() override {
    return std::string(memory.begin(), memory.end());
}

void FirstFit::initializeMemory() {
    std::fill(memory.begin(), memory.end(), '.'); // '.' represents unallocated memory
    std::fill(allocationMap.begin(), allocationMap.end(), false);
}

bool FirstFit::canAllocateAt(size_t index, size_t size) const {
    return (index + size <= maximumSize);
}

void FirstFit::allocateAt(size_t index, size_t size) {
    std::fill(allocationMap.begin() + index, allocationMap.begin() + index + size, true);
}

void FirstFit::deallocateAt(size_t index) {
    allocationMap[index] = false;
}



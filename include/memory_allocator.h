#ifndef IMEMORY_ALLOCATOR_H
#define IMEMORY_ALLOCATOR_H
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace allocator {
    struct AllocationRecord {
        size_t startBlock; // Starting block index of the allocation
        size_t size;       // Size of the allocation in bytes or blocks
        std::string name;  // Name of process that owns the allocation
    };

    class IMemoryAllocator {
    public:
        virtual void* allocate(size_t size, const std::string &name) = 0;
        virtual void deallocate(void* ptr) = 0;
        virtual std::string visualizeMemory() = 0;

    protected:
        size_t blockSize;
    };

    class FirstFit : public IMemoryAllocator {
    public:
        FirstFit(size_t size, size_t blockSize);
        ~FirstFit();

        void* allocate(size_t size, const std::string& name);
        void deallocate(void* ptr);
        std::string visualizeMemory();
    private:
        size_t maximumSize;
        size_t allocatedSize;
        std::vector<char> memory;
        std::vector<AllocationRecord> allocations;
        std::mutex allocatorMutex;
        void initializeMemory();
        bool canAllocateAt(size_t index, size_t size) const;
        bool isBlockFree(size_t startBlock, size_t numBlocks) const;
        void allocateAt(size_t index, size_t size);
        void deallocateAt(size_t index);
    };
}
#endif //IMEMORY_ALLOCATOR_H

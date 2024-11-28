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
        size_t entranceCycle = 0;
    };

    struct Page {
        size_t index; // Page number
        std::string name; // Name of process that owns the page
        size_t entranceCycle; // CPU Cycle when the page was allocated
    };
    class IMemoryAllocator {
    public:
        // virtual void* allocate(size_t size, const std::string &name) = 0;
        // virtual void deallocate(void* ptr) = 0;
        virtual std::string visualizeMemory() = 0;
        virtual void moveToBackingStore(const std::string& name) = 0;
        // void getFromBackingStore(std::string name);
        // virtual bool canAllocate(size_t size);
        virtual bool inBackingStore(std::string name) = 0;
        // virtual bool isAllocated(std::string name);
        virtual std::string getOldestProcessNotRunning(std::vector<std::string> running) = 0;
    protected:
        size_t blockSize = 0;
        size_t maximumSize = 0;
        size_t allocatedSize = 0;
    };

    class FlatModel : public IMemoryAllocator {
    public:
        FlatModel(size_t size, size_t blockSize);
        ~FlatModel();

        void* allocate(size_t size, const std::string& name, size_t entranceCycle);
        void deallocate(void* ptr);
        std::string visualizeMemory();
        // bool canAllocate(size_t size);
        bool inBackingStore(std::string name);
        // bool isAllocated(std::string name);
        std::string getOldestProcessNotRunning(std::vector<std::string> running);
        void moveToBackingStore(const std::string& name);
        void* getFromBackingStore(const std::string& name, size_t entranceCycle);
    private:
        size_t maximumSize;
        size_t allocatedSize;
        std::vector<char> memory;
        std::vector<AllocationRecord> allocations;
        std::mutex allocatedMutex;
        std::vector<AllocationRecord> backingStore;
        std::mutex backingStoreMutex;
        void initializeMemory();
        bool canAllocateAt(size_t index, size_t size) const;
        bool isBlockFree(size_t startBlock, size_t numBlocks) const;
        // void allocateAt(size_t index, size_t size);
        // void deallocateAt(size_t index);
    };

    class Paging : public IMemoryAllocator {
    public:
        Paging(size_t size, size_t pageSize);
        ~Paging();

        void *allocate(size_t size, const std::string &name, size_t entranceCycle);
        void deallocate(const std::string& name);
        std::string visualizeMemory();
        void initializeMemory();
        void moveToBackingStore(const std::string& name);
        void getFromBackingStore(const std::string& name, size_t entranceCycle);
        bool canAllocate(size_t size);
        bool inBackingStore(std::string name);
        bool isAllocated(std::string name);
        std::string getOldestProcessNotRunning(std::vector<std::string> running);
        size_t getTotalIn();
        size_t getTotalOut();
    private:
        size_t maximumSize;
        size_t allocatedSize;
        size_t pageSize;
        std::vector<char> memory;
        std::vector<Page> freePages;
        std::vector<Page> allocatedPages;
        std::vector<Page> backingStore;
        std::mutex freeMutex;
        std::mutex allocatedMutex;
        std::mutex backingStoreMutex;
        size_t totalPagedIn = 0;
        size_t totalPagedOut = 0;
    };
}
#endif //IMEMORY_ALLOCATOR_H

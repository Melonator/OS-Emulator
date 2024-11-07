#ifndef IMEMORY_ALLOCATOR_H
#define IMEMORY_ALLOCATOR_H
#include <string>
#include <unordered_map>
#include <vector>

namespace allocator {
    class IMemoryAllocator {
    public:
        virtual void* allocate(size_t size) = 0;
        virtual void deallocate(void* ptr) = 0;
        virtual std::string visualizeMemory() = 0;
    };

    class FirstFit : public IMemoryAllocator {
    public:
        FirstFit(size_t size);
        ~FirstFit();

        void* allocate(size_t size) override;
        void deallocate(void* ptr) override;
        std::string visualizeMemory() override;
    private:
        size_t maximumSize;
        size_t allocatedSize;
        std::vector<char> memory;
        std::unordered_map<size_t, bool> allocationMap;
        void initializeMemory();
        bool canAllocateAt(size_t index, size_t size) const;
        void allocateAt(size_t index, size_t size);
        void deallocateAt(size_t index);
    };
}
#endif //IMEMORY_ALLOCATOR_H

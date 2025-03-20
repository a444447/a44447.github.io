#include <iostream>
#include <vector>

class MemoryPool {

    private:
        struct Block {
            Block* next;
        };

        Block* freelist;
        std::vector<void*> chunks;
        size_t blockSize;
        size_t chunkSize;

        void allocateChunk() {
            size_t chunkBytes = blockSize * chunkSize;
            void* chunk = malloc(chunkBytes);
            chunks.push_back(chunk);

            char* start = static_cast<char*>(chunk);
            for (size_t i = 0; i < chunkSize; ++i) {
                Block* block = reinterpret_cast<Block*>(start + i * blockSize);
                block->next = freelist;
                freelist = block;
            }

        }
    public:
        MemoryPool(size_t blockSize, size_t chunkSize = 32) : freelist(nullptr), blockSize(blockSize), chunkSize(chunkSize) {}
        ~MemoryPool() {
            for (void* chunk : chunks) {
                free(chunk);
            }
        }

        void* allocate() {
            if (!freelist) {
                allocateChunk();
            }
            Block* block = freelist;
            freelist = freelist->next;
            return block;
        }

};
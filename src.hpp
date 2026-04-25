
#include <vector>
#include <algorithm>

/**
 * Get specified 4096 * n bytes from the memory.
 * @param n
 * @return the address of the block
 */
int* getNewBlock(int n);

/**
 * Free specified 4096 * n bytes from the memory.
 * @param block the pointer to the block
 * @param n
 */
void freeBlock(const int* block, int n);

class Allocator {
private:
    struct Block {
        int* ptr;
        int n_blocks;
    };

    struct FreeSegment {
        int* ptr;
        int size;
        bool operator<(const FreeSegment& other) const {
            return ptr < other.ptr;
        }
    };

    std::vector<Block> blocks;
    std::vector<FreeSegment> free_list;
    
    int* current_block_ptr;
    int current_block_remaining;

    void merge_free_list() {
        if (free_list.empty()) return;
        std::sort(free_list.begin(), free_list.end());
        std::vector<FreeSegment> merged;
        merged.push_back(free_list[0]);
        for (size_t i = 1; i < free_list.size(); ++i) {
            if (merged.back().ptr + merged.back().size == free_list[i].ptr) {
                merged.back().size += free_list[i].size;
            } else {
                merged.push_back(free_list[i]);
            }
        }
        free_list = std::move(merged);
    }

public:
    Allocator() : current_block_ptr(nullptr), current_block_remaining(0) {}

    ~Allocator() {
        for (const auto& block : blocks) {
            freeBlock(block.ptr, block.n_blocks);
        }
    }

    int* allocate(int n) {
        // 1. Try to find in free_list
        for (auto it = free_list.begin(); it != free_list.end(); ++it) {
            if (it->size >= n) {
                int* res = it->ptr;
                if (it->size > n) {
                    it->ptr += n;
                    it->size -= n;
                } else {
                    free_list.erase(it);
                }
                return res;
            }
        }

        // 2. Try to use current block
        if (n <= current_block_remaining) {
            int* res = current_block_ptr;
            current_block_ptr += n;
            current_block_remaining -= n;
            return res;
        }

        // 3. Allocate new block
        int ints_per_block = 4096 / sizeof(int);
        int n_blocks_needed = (n + ints_per_block - 1) / ints_per_block;
        int* new_block_ptr = getNewBlock(n_blocks_needed);
        
        blocks.push_back({new_block_ptr, n_blocks_needed});

        int total_ints = n_blocks_needed * ints_per_block;
        int* res = new_block_ptr;
        
        current_block_ptr = new_block_ptr + n;
        current_block_remaining = total_ints - n;

        return res;
    }

    void deallocate(int* pointer, int n) {
        if (!pointer) return;
        free_list.push_back({pointer, n});
        merge_free_list();
    }
};

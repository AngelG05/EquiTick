#pragma once

#include <vector>
#include <mutex>
#include <cassert>
#include <cstdint>

namespace equitick {

/**
 * @brief Thread-safe Fixed-Size Memory Pool.
 * 
 * Pre-allocates memory for core engine structs to eliminate the jitter
 * caused by OS-level memory allocation (malloc/free) in the hot path.
 * 
 * @tparam T The type to allocate.
 * @tparam PoolSize Maximum number of objects in the pool.
 */
template <typename T, size_t PoolSize>
class MemoryPool {
public:
    MemoryPool() {
        pool_.resize(PoolSize);
        free_indices_.reserve(PoolSize);
        for (size_t i = 0; i < PoolSize; ++i) {
            free_indices_.push_back(i);
        }
    }

    /** @return Pointer to a free object, or nullptr if pool is exhausted. */
    T* allocate() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (free_indices_.empty()) [[unlikely]] return nullptr;
        
        size_t index = free_indices_.back();
        free_indices_.pop_back();
        return &pool_[index];
    }

    /** @brief Returns an object to the pool. */
    void deallocate(T* ptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        uintptr_t diff = reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(&pool_[0]);
        size_t index = diff / sizeof(T);
        assert(index < PoolSize);
        free_indices_.push_back(index);
    }

private:
    std::vector<T> pool_;
    std::vector<size_t> free_indices_;
    std::mutex mutex_; 
};

} // namespace equitick

#pragma once

#include <atomic>
#include <vector>
#include <optional>
#include <cassert>

namespace equitick {

/**
 * @brief A lock-free Multi-Producer Single-Consumer (MPSC) Ring Buffer.
 * 
 * Designed for ultra-low latency event passing between feed handlers and the engine.
 * Uses atomic compare-and-swap for thread-safe multi-producer support.
 * 
 * @tparam T The type of elements stored in the buffer.
 * @tparam Capacity Must be a power of 2 for optimized wrap-around logic.
 */
template <typename T, size_t Capacity>
class RingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2 for fast masking");

public:
    RingBuffer() : head_(0), tail_(0) {
        buffer_.resize(Capacity);
    }

    /**
     * @brief Pushes an item into the buffer (Multi-producer safe).
     * 
     * Uses Acquire-Release semantics to ensure visibility across threads.
     * @return true if pushed, false if buffer is full.
     */
    bool push(const T& item) {
        size_t head = head_.load(std::memory_order_relaxed);
        do {
            // Check for full condition: head - tail == Capacity
            // We use acquire to see the latest tail from the consumer.
            if (head - tail_.load(std::memory_order_acquire) == Capacity) {
                return false; 
            }
        } while (!head_.compare_exchange_weak(head, head + 1, 
                                              std::memory_order_release, 
                                              std::memory_order_relaxed));
        
        buffer_[head & (Capacity - 1)] = item;
        return true;
    }

    /**
     * @brief Pops an item from the buffer (Single-consumer only).
     * 
     * @return std::optional containing the item, or nullopt if empty.
     */
    std::optional<T> pop() {
        size_t tail = tail_.load(std::memory_order_relaxed);
        // If tail == head, buffer is empty.
        if (tail == head_.load(std::memory_order_acquire)) {
            return std::nullopt; 
        }
        
        T item = buffer_[tail & (Capacity - 1)];
        tail_.store(tail + 1, std::memory_order_release);
        return item;
    }

private:
    alignas(64) std::atomic<size_t> head_; ///< Atomic producer index
    alignas(64) std::atomic<size_t> tail_; ///< Atomic consumer index
    std::vector<T> buffer_;                ///< Underlying storage
};

} // namespace equitick

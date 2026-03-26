#pragma once

#include <thread>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#endif

namespace equitick {

/**
 * @brief Hardware-level optimization utilities.
 * 
 * Provides methods for NUMA awareness and core pinning to minimize
 * context-switching overhead in high-throughput streaming systems.
 */
class HardwareUtils {
public:
    /**
     * @brief Pins the current thread to a specific CPU core.
     * 
     * @param core_id The index of the core to pin to.
     */
    static void pin_to_core(size_t core_id) {
#ifdef _WIN32
        HANDLE thread = GetCurrentThread();
        DWORD_PTR mask = static_cast<DWORD_PTR>(1) << core_id;
        SetThreadAffinityMask(thread, mask);
#else
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core_id, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
#endif
    }

    /**
     * @brief Hints the CPU to yield without entering a deep sleep state.
     * Ideal for spin-waiting on lock-free queues.
     */
    static void cpu_relax() {
#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
    #if defined(_MSC_VER)
        _mm_pause();
    #else
        __builtin_ia32_pause();
    #endif
#elif defined(__arm__) || defined(__aarch64__)
        asm volatile("yield" ::: "memory");
#endif
    }
};

} // namespace equitick

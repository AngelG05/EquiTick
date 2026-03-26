/**
 * @file main.cpp
 * @brief EquiTick High-Frequency Pay Equity Enforcement Engine
 *
 * Core engine implementation orchestrating zero-copy feed handling, 
 * real-time regression, and automated fairness interception.
 */

#include "types.hpp"
#include "ring_buffer.hpp"
#include "online_regression.hpp"
#include "anomaly_detector.hpp"
#include "interceptor.hpp"
#include "hardware_utils.hpp"
#include "simd_math.hpp"
#include "memory_pool.hpp"

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <random>

using namespace equitick;

/** @brief Global shutdown signal for the engine and producer threads. */
std::atomic<bool> g_keep_running{true};

/**
 * @brief The Core Engine Thread.
 * 
 * Continuously drains the lock-free MPSC ring buffer, updates fairness models,
 * and performs sub-microsecond interception on compensation decisions.
 */
void engine_worker(RingBuffer<CompensationEvent, 65536>& queue) {
    // Optimization: Pin this thread to a dedicated physical core to eliminate context switching.
    HardwareUtils::pin_to_core(1);

    OnlineRegression model;
    AnomalyDetector detector;
    Interceptor interceptor(0.2, 3.5);

    const size_t SIMD_BATCH_SIZE = 128;
    std::vector<double> adjustment_batch;
    adjustment_batch.reserve(SIMD_BATCH_SIZE);

    uint64_t processed_count = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    std::cout << "[INFO] Engine worker started on Core 1. Monitoring stream..." << std::endl;

    while (g_keep_running || queue.pop()) {
        auto event_opt = queue.pop();
        if (!event_opt) {
            HardwareUtils::cpu_relax(); // Low-latency wait without yielding to OS
            continue;
        }

        const auto& event = *event_opt;
        auto process_start = std::chrono::high_resolution_clock::now();

        // 1. Online Regression Model Update (O(1))
        // We normalize pay against Years of Experience.
        model.update(static_cast<double>(event.YearsExperience), event.BaseSalary);

        // 2. Anomaly Detection (Statistical Arbitrage logic applied to pay)
        double predicted = model.predict(event.YearsExperience);
        detector.add_observation(event.BaseSalary);
        double z_score = detector.get_z_score(event.BaseSalary);

        // 3. Fairness Interception Layer
        // Decide whether to block or approve the compensation event.
        Decision decision = interceptor.process(event, predicted, z_score);

        // 4. Vectorized Batch Processing (SIMD Demo)
        // In a real system, this would be used for batch currency conversion or tiered adjustment.
        adjustment_batch.push_back(event.BaseSalary);
        if (adjustment_batch.size() >= SIMD_BATCH_SIZE) {
            SIMDMath::add_batch(adjustment_batch.data(), SIMD_BATCH_SIZE, 0.0); 
            adjustment_batch.clear();
        }

        auto process_end = std::chrono::high_resolution_clock::now();
        decision.processing_latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(process_end - process_start).count();

        processed_count++;

        // High-frequency periodic reporting
        if (processed_count % 100000 == 0) [[unlikely]] {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start_time).count();
            
            std::cout << "\r[EquiTick] " 
                      << "Throughput: " << std::fixed << std::setprecision(0) << (processed_count / elapsed) << " evt/s | "
                      << "Blocked: " << std::setw(4) << interceptor.get_blocked_count() << " | "
                      << "Latency: " << std::setw(4) << decision.processing_latency_ns << "ns" << std::flush;
        }
    }
    std::cout << std::endl << "[INFO] Engine worker gracefully shutting down." << std::endl;
}

/**
 * @brief High-Frequency Event Producer (Feed Handler Simulator).
 * 
 * Simulates a high-speed stream of compensation decision events coming from
 * an HR Information System (HRIS) or recruitment pipeline.
 */
void feed_handler_sim(RingBuffer<CompensationEvent, 65536>& queue) {
    HardwareUtils::pin_to_core(2); // Pin producer to Core 2

    uint64_t event_id = 0;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> salary_dist(60000, 150000);
    std::uniform_int_distribution<int> exp_dist(0, 25);
    std::uniform_int_distribution<int> bias_roll(1, 100);

    while (g_keep_running) {
        CompensationEvent event;
        event.event_id = event_id++;
        event.BaseSalary = static_cast<double>(salary_dist(rng));
        event.YearsExperience = static_cast<uint16_t>(exp_dist(rng));
        
        // Dynamic Bias Simulation:
        if (bias_roll(rng) <= 10) {
            event.gender = Gender::FEMALE;
            event.BaseSalary *= 0.85;
        } else {
            event.gender = Gender::MALE;
        }

        // Push to lock-free queue with spin-retry if full
        while (!queue.push(event)) {
            HardwareUtils::cpu_relax();
        }

        if (event_id >= 2000000) break; // Limit simulation to 2M events
    }
    g_keep_running = false;
}

int main() {
    std::cout << " EquiTick: Real-Time Pay Equity Enforcement Engine" << std::endl;

    // Use a large enough buffer to handle bursts in traffic
    RingBuffer<CompensationEvent, 65536> event_queue;

    // Spawn high-priority threads
    std::thread engine(engine_worker, std::ref(event_queue));
    std::thread producer(feed_handler_sim, std::ref(event_queue));

    producer.join();
    engine.join();

    std::cout << "Engine finalized processing. All fairness constraints enforced." << std::endl;
    return 0;
}

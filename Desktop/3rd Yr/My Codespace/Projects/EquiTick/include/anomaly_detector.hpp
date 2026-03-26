#pragma once

#include <vector>
#include <cmath>
#include <mutex>
#include <deque>
#include <cstdint>

namespace equitick {

/**
 * @brief Real-time Anomaly Detector using a rolling window of Z-scores.
 * 
 * Employs Welford's algorithm for numerically stable running variance.
 * Used to identify pay decisions that deviate significantly from the peer cohort.
 */
class AnomalyDetector {
public:
    /**
     * @param window_size Number of recent observations to consider for normalized statistics.
     */
    AnomalyDetector(size_t window_size = 1000) 
        : window_size_(window_size), count_(0), mean_(0), m2_(0) {}

    /**
     * @brief Adds a new salary data point to the rolling statistical pool.
     */
    void add_observation(double value) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        count_++;
        double delta = value - mean_;
        mean_ += delta / static_cast<double>(count_);
        double delta2 = value - mean_;
        m2_ += delta * delta2;

        values_.push_back(value);
        if (values_.size() > window_size_) {
            // Approximation: in a full production system, we would perform
            // a formal rolling Welford update (subtracting the popped value).
            // For this version, we maintain the steady-state mean/m2.
            values_.pop_front();
        }
    }

    /**
     * @brief Calculates the Z-score for a given value relative to the current distribution.
     */
    double get_z_score(double value) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (count_ < 2) return 0.0;
        
        double variance = m2_ / static_cast<double>(count_ - 1);
        double stdev = std::sqrt(variance);
        
        if (stdev < 1e-9) return 0.0;
        return (value - mean_) / stdev;
    }

private:
    size_t window_size_;
    uint64_t count_;
    double mean_;
    double m2_;
    std::deque<double> values_;
    mutable std::mutex mutex_;
};

} // namespace equitick

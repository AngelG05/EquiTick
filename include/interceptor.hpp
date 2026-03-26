#pragma once

#include "types.hpp"
#include <string>
#include <cstring>
#include <cmath>
#include <cstdint>

namespace equitick {

/**
 * @brief High-frequency Interception Layer.
 * 
 * Enforces fairness constraints in real-time. If the bias score or statistical
 * anomaly (Z-score) exceeds safety thresholds, the decision is blocked.
 */
class Interceptor {
public:
    /**
     * @param bias_threshold Percentage deviation allowed before blocking (e.g., 0.2 = 20%).
     * @param z_threshold Number of standard deviations allowed for statistical normality.
     */
    Interceptor(double bias_threshold = 0.2, double z_threshold = 3.0)
        : bias_threshold_(bias_threshold), z_threshold_(z_threshold), blocked_count_(0) {}

    /**
     * @brief Processes an event and returns a formal decision.
     * 
     * Uses `[[likely]]` / `[[unlikely]]` to guide the CPU branch predictor, 
     * a common HFT optimization for the "happy path" (approved decisions).
     * 
     * @param event The original compensation event.
     * @param predicted_salary The value from the regression model for this peer group.
     * @param z_score The statistical anomaly score.
     * @return Decision status and metadata.
     */
    Decision process(const CompensationEvent& event, double predicted_salary, double z_score) {
        Decision decision;
        decision.event_id = event.event_id;
        
        // Calculate the Bias Score as the relative deviation from the fair market prediction.
        decision.BiasScore = (event.BaseSalary - predicted_salary) / (predicted_salary + 1e-9);
        decision.ZScore = z_score;
        decision.Blocked = false;

        // Threshold-based enforcement:
        // We block if the salary is significantly LOWER than predicted (bias against)
        // or if it's a massive statistical outlier (Z-score > threshold).
        if (decision.BiasScore < -bias_threshold_ || std::abs(z_score) > z_threshold_) [[unlikely]] {
            decision.Blocked = true;
            std::strncpy(decision.Reason, "FAIRNESS_VIOLATION_DETECTED", sizeof(decision.Reason));
            blocked_count_++;
        } else [[likely]] {
            std::strncpy(decision.Reason, "APPROVED", sizeof(decision.Reason));
        }

        return decision;
    }

    /** @return Total number of blocked inequitable decisions since engine start. */
    uint64_t get_blocked_count() const { return blocked_count_; }

private:
    double bias_threshold_;
    double z_threshold_;
    uint64_t blocked_count_;
};

} // namespace equitick

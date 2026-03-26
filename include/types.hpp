#pragma once

#include <cstdint>
#include <chrono>
#include <array>

namespace equitick {

/**
 * @brief Categorical representation of gender for bias analysis.
 */
enum class Gender : uint8_t {
    MALE = 0,
    FEMALE = 1,
    NON_BINARY = 2,
    PREFER_NOT_TO_SAY = 3
};

/**
 * @brief High-frequency compensation event structure.
 * 
 * Aligned to 64 bytes (typical x86 cache line) to prevent false sharing
 * and maximize L1 cache hit rates during streaming ingestion.
 */
struct alignas(64) CompensationEvent {
    uint64_t event_id;          ///< Unique identifier for the event
    uint64_t timestamp_ns;      ///< High-resolution timestamp in nanoseconds
    uint64_t employee_id;       ///< unique employee identifier
    double BaseSalary;          ///< Proposed annual base salary
    double Bonus;               ///< Proposed annual bonus
    uint16_t YearsExperience;   ///< Total years of relevant professional experience
    uint16_t Level;             ///< Organization hierarchy level
    Gender gender;              ///< Protected characteristic for fairness analysis
    char RoleID[8];             ///< Compressed role identifier (zero-copy friendly)
    char RegionID[4];           ///< Compressed region identifier
};

/**
 * @brief Output decision of the interception layer.
 * 
 * Contains bias metrics and the final block/approve status.
 */
struct alignas(64) Decision {
    uint64_t event_id;              ///< Original event identifier
    uint64_t processing_latency_ns; ///< End-to-end engine processing time
    double BiasScore;               ///< Normalized deviation from peer regression
    double ZScore;                  ///< Statistical anomaly score
    bool Blocked;                   ///< True if the decision was blocked by the kill-switch
    char Reason[31];                ///< Human-readable reason for the decision
};

} // namespace equitick

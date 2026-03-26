#pragma once

#if defined(__AVX2__) || defined(_M_AMD64) || defined(_M_X64)
#include <immintrin.h>
#endif

#include <vector>
#include <cstdint>
#include <cstddef>

namespace equitick {

/**
 * @brief Static kernel library for Vectorized (SIMD) Math.
 * 
 * Optimized for x86_64 architectures with AVX2 support. Fallback to scalar
 * loops provided for non-AVX systems.
 */
class SIMDMath {
public:
    /**
     * @brief Batch salary adjustments using AVX2.
     * 
     * @param data Pointer to the buffer of double-precision values.
     * @param size Number of elements (should be a multiple of 4 for peak performance).
     * @param value The scalar value to add to every element.
     */
    static void add_batch(double* data, size_t size, double value) {
#if defined(__AVX2__) || defined(_M_X64)
        __m256d v_val = _mm256_set1_pd(value);
        size_t i = 0;
        // Process 4 doubles at a time
        for (; i + 3 < size; i += 4) {
            __m256d v_data = _mm256_loadu_pd(&data[i]);
            v_data = _mm256_add_pd(v_data, v_val);
            _mm256_storeu_pd(&data[i], v_data);
        }
        // Scalar fallback for remaining elements
        for (; i < size; ++i) data[i] += value;
#else
        for (size_t i = 0; i < size; ++i) data[i] += value;
#endif
    }

    /**
     * @brief Calculates the sum of squares for a batch of values.
     * 
     * Used for real-time variance and standard deviation calculations.
     */
    static double sum_squares(const double* data, size_t size) {
        double result = 0;
#if defined(__AVX2__) || defined(_M_X64)
        __m256d v_sum = _mm256_setzero_pd();
        size_t i = 0;
        for (; i + 3 < size; i += 4) {
            __m256d v_data = _mm256_loadu_pd(&data[i]);
            v_sum = _mm256_add_pd(v_sum, _mm256_mul_pd(v_data, v_data));
        }
        alignas(32) double temp[4];
        _mm256_storeu_pd(temp, v_sum);
        result = temp[0] + temp[1] + temp[2] + temp[3];
        for (; i < size; ++i) result += data[i] * data[i];
#else
        for (size_t i = 0; i < size; ++i) result += data[i] * data[i];
#endif
        return result;
    }
};

} // namespace equitick

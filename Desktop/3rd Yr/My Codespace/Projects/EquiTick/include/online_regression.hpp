#pragma once

#include <cmath>
#include <mutex>
#include <cstdint>

namespace equitick {

/**
 * @brief Online Regression Engine using incremental Ordinary Least Squares (OLS).
 * 
 * Provides O(1) updates for real-time pay normalization. 
 * Allows the engine to detect bias variations dynamically as market data flows in.
 */
class OnlineRegression {
public:
    OnlineRegression() : n_(0), sum_x_(0), sum_y_(0), sum_xx_(0), sum_xy_(0) {}

    /**
     * @brief Incorporates a new salary observation into the model.
     * 
     * @param x The independent variable (e.g., Years of Experience).
     * @param y The dependent variable (e.g., Annual Salary).
     */
    void update(double x, double y) {
        std::lock_guard<std::mutex> lock(mutex_);
        n_++;
        sum_x_ += x;
        sum_y_ += y;
        sum_xx_ += x * x;
        sum_xy_ += x * y;
    }

    /**
     * @brief Predicts the expected salary for a given experience level based on the current model.
     * 
     * @param x Years of Experience.
     * @return double Predicted fair salary.
     */
    double predict(double x) const {
        double beta, alpha;
        get_coefficients(alpha, beta);
        return alpha + beta * x;
    }

    /**
     * @brief Calculates the current alpha (intercept) and beta (slope).
     */
    void get_coefficients(double& alpha, double& beta) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (n_ < 2) {
            alpha = 0;
            beta = 0;
            return;
        }

        double denominator = (static_cast<double>(n_) * sum_xx_ - sum_x_ * sum_x_);
        if (std::abs(denominator) < 1e-9) {
            alpha = sum_y_ / static_cast<double>(n_);
            beta = 0;
        } else {
            beta = (static_cast<double>(n_) * sum_xy_ - sum_x_ * sum_y_) / denominator;
            alpha = (sum_y_ - beta * sum_x_) / static_cast<double>(n_);
        }
    }

private:
    uint64_t n_;        ///< Number of observations
    double sum_x_;      ///< Cumulative sum of experience
    double sum_y_;      ///< Cumulative sum of salaries
    double sum_xx_;     ///< Cumulative sum of squares of experience
    double sum_xy_;     ///< Cumulative sum of products (x * y)
    mutable std::mutex mutex_; ///< Thread-safety for model updates
};

} // namespace equitick

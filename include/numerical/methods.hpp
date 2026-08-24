#pragma once

#include "ast/expression.hpp"

#include <cstddef>
#include <functional>
#include <string>

namespace calcx::numerical {

struct NumericalResult {
    double value;
    std::size_t iterations;
    std::string method;
};

NumericalResult trapezoidal(const ast::ExpressionPtr& expression, const std::string& variable,
                            double lower, double upper, std::size_t intervals = 1000);
NumericalResult simpson(const ast::ExpressionPtr& expression, const std::string& variable,
                        double lower, double upper, std::size_t intervals = 1000);
NumericalResult derivative(const ast::ExpressionPtr& expression, const std::string& variable,
                           double point, double step = 1e-6);
NumericalResult newton_raphson(const std::function<double(double)>& function,
                               const std::function<double(double)>& derivative,
                               double initial, double tolerance = 1e-10,
                               std::size_t max_iterations = 100);
NumericalResult bisection(const std::function<double(double)>& function, double lower, double upper,
                          double tolerance = 1e-10, std::size_t max_iterations = 100);

}  // namespace calcx::numerical

#include "numerical/methods.hpp"

#include <cmath>
#include <stdexcept>
#include <unordered_map>

namespace calcx::numerical {
namespace {

void check_intervals(std::size_t intervals, bool require_even = false) {
    if (intervals == 0 || (require_even && intervals % 2 != 0)) {
        throw std::invalid_argument(require_even ? "Simpson integration requires an even interval count." : "Interval count must be positive.");
    }
}

double value_at(const ast::ExpressionPtr& expression, const std::string& variable, double point) {
    return expression->evaluate({{variable, point}});
}

}  // namespace

NumericalResult trapezoidal(const ast::ExpressionPtr& expression, const std::string& variable,
                            double lower, double upper, std::size_t intervals) {
    check_intervals(intervals);
    const double width = (upper - lower) / static_cast<double>(intervals);
    double sum = 0.5 * (value_at(expression, variable, lower) + value_at(expression, variable, upper));
    for (std::size_t index = 1; index < intervals; ++index) sum += value_at(expression, variable, lower + index * width);
    return {sum * width, intervals, "trapezoidal rule"};
}

NumericalResult simpson(const ast::ExpressionPtr& expression, const std::string& variable,
                        double lower, double upper, std::size_t intervals) {
    check_intervals(intervals, true);
    const double width = (upper - lower) / static_cast<double>(intervals);
    double sum = value_at(expression, variable, lower) + value_at(expression, variable, upper);
    for (std::size_t index = 1; index < intervals; ++index) {
        sum += (index % 2 == 0 ? 2.0 : 4.0) * value_at(expression, variable, lower + index * width);
    }
    return {sum * width / 3.0, intervals, "Simpson's rule"};
}

NumericalResult derivative(const ast::ExpressionPtr& expression, const std::string& variable,
                           double point, double step) {
    if (step <= 0) throw std::invalid_argument("Derivative step must be positive.");
    const double result = (value_at(expression, variable, point + step) - value_at(expression, variable, point - step)) / (2.0 * step);
    return {result, 1, "central difference"};
}

NumericalResult newton_raphson(const std::function<double(double)>& function,
                               const std::function<double(double)>& derivative_function,
                               double initial, double tolerance, std::size_t max_iterations) {
    double current = initial;
    for (std::size_t iteration = 1; iteration <= max_iterations; ++iteration) {
        const double slope = derivative_function(current);
        if (std::abs(slope) < 1e-14) throw std::runtime_error("Newton-Raphson encountered a near-zero derivative.");
        const double next = current - function(current) / slope;
        if (std::abs(next - current) <= tolerance) return {next, iteration, "Newton-Raphson"};
        current = next;
    }
    throw std::runtime_error("Newton-Raphson did not converge within the iteration limit.");
}

NumericalResult bisection(const std::function<double(double)>& function, double lower, double upper,
                          double tolerance, std::size_t max_iterations) {
    double left_value = function(lower);
    const double right_value = function(upper);
    if (left_value * right_value > 0) throw std::invalid_argument("Bisection endpoints must bracket a root.");
    for (std::size_t iteration = 1; iteration <= max_iterations; ++iteration) {
        const double midpoint = (lower + upper) / 2.0;
        const double midpoint_value = function(midpoint);
        if (std::abs(midpoint_value) <= tolerance || std::abs(upper - lower) <= tolerance) return {midpoint, iteration, "bisection"};
        if (left_value * midpoint_value <= 0) upper = midpoint;
        else { lower = midpoint; left_value = midpoint_value; }
    }
    throw std::runtime_error("Bisection did not converge within the iteration limit.");
}

}  // namespace calcx::numerical

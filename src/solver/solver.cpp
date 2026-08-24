#include "solver/solver.hpp"

#include "calculus/differentiation.hpp"

#include <cmath>
#include <limits>
#include <experimental/optional>

namespace calcx::solver {
namespace {
using ast::BinaryOperator;
using ast::Expression;
using ast::ExpressionPtr;
using ast::UnaryOperator;
using Polynomial = std::vector<double>;

Polynomial add_poly(const Polynomial& left, const Polynomial& right, double sign = 1.0) {
    Polynomial result(std::max(left.size(), right.size()), 0.0);
    for (std::size_t i = 0; i < result.size(); ++i) {
        if (i < left.size()) result[i] += left[i];
        if (i < right.size()) result[i] += sign * right[i];
    }
    return result;
}
std::experimental::optional<Polynomial> polynomial(const ExpressionPtr& expression, const std::string& variable) {
    if (expression->kind() == Expression::Kind::Number) return Polynomial{expression->number_value()};
    if (expression->kind() == Expression::Kind::Variable) {
        if (expression->variable_name() == variable) return Polynomial{0.0, 1.0};
        return std::experimental::nullopt;
    }
    if (expression->kind() == Expression::Kind::Unary) {
        auto value = polynomial(expression->operand(), variable); if (!value) return std::experimental::nullopt;
        if (expression->unary_operator() == UnaryOperator::Negate) for (double& coefficient : *value) coefficient = -coefficient;
        return value;
    }
    if (expression->kind() != Expression::Kind::Binary) return std::experimental::nullopt;
    auto left = polynomial(expression->left(), variable); auto right = polynomial(expression->right(), variable);
    if (expression->binary_operator() == BinaryOperator::Add || expression->binary_operator() == BinaryOperator::Subtract) {
        if (!left || !right) return std::experimental::nullopt;
        return add_poly(*left, *right, expression->binary_operator() == BinaryOperator::Add ? 1.0 : -1.0);
    }
    if (expression->binary_operator() == BinaryOperator::Multiply && left && right && left->size() + right->size() <= 4) {
        Polynomial result(left->size() + right->size() - 1, 0.0);
        for (std::size_t i = 0; i < left->size(); ++i) for (std::size_t j = 0; j < right->size(); ++j) result[i + j] += (*left)[i] * (*right)[j];
        return result;
    }
    if (expression->binary_operator() == BinaryOperator::Power && left && expression->right()->kind() == Expression::Kind::Number) {
        const double exponent = expression->right()->number_value();
        if (exponent >= 0 && exponent <= 3 && std::floor(exponent) == exponent) {
            Polynomial result{1.0};
            if (exponent == 0) return result;
            result = *left;
            for (int count = 1; count < static_cast<int>(exponent); ++count) {
                Polynomial next(result.size() + left->size() - 1, 0.0);
                for (std::size_t i = 0; i < result.size(); ++i) for (std::size_t j = 0; j < left->size(); ++j) next[i + j] += result[i] * (*left)[j];
                result = next;
            }
            return result;
        }
    }
    return std::experimental::nullopt;
}
void trim(Polynomial& coefficients) { while (coefficients.size() > 1 && std::abs(coefficients.back()) < 1e-12) coefficients.pop_back(); }

double evaluate_at(const ExpressionPtr& expression, const std::string& variable, double point) { return expression->evaluate({{variable, point}}); }
}  // namespace

EquationResult solve_equation(const ast::ExpressionPtr& left, const ast::ExpressionPtr& right, const std::string& variable) {
    auto left_poly = polynomial(left, variable); auto right_poly = polynomial(right, variable);
    if (!left_poly || !right_poly) return {false, "unsupported", {}, "Only polynomial equations up to degree two are currently supported."};
    auto coefficients = add_poly(*left_poly, *right_poly, -1.0); trim(coefficients);
    if (coefficients.size() == 1) return {std::abs(coefficients[0]) < 1e-12, "constant equation", {}, std::abs(coefficients[0]) < 1e-12 ? "Every value is a solution." : "No solution."};
    if (coefficients.size() == 2) return {true, "linear formula", {std::complex<double>(-coefficients[0] / coefficients[1], 0.0)}, ""};
    const double discriminant = coefficients[1] * coefficients[1] - 4.0 * coefficients[2] * coefficients[0];
    const std::complex<double> root = std::sqrt(std::complex<double>(discriminant, 0.0));
    return {true, "quadratic formula", {(-coefficients[1] + root) / (2.0 * coefficients[2]), (-coefficients[1] - root) / (2.0 * coefficients[2])}, ""};
}

LimitResult limit(const ast::ExpressionPtr& expression, const std::string& variable, double point) {
    try {
        const double direct = evaluate_at(expression, variable, point);
        if (std::isfinite(direct)) return {true, false, direct, "direct substitution", ""};
    } catch (const ast::EvaluationError&) { }
    auto simplified = calculus::simplify(expression);
    try {
        const double value = evaluate_at(simplified, variable, point);
        if (std::isfinite(value)) return {true, false, value, "simplification", ""};
    } catch (const ast::EvaluationError&) { }
    auto derivative = calculus::differentiate(expression, variable).expression;
    try {
        const double value = evaluate_at(derivative, variable, point);
        if (std::isfinite(value)) return {true, false, value, "L'Hopital's rule", ""};
    } catch (const ast::EvaluationError&) { }
    return {false, false, std::numeric_limits<double>::quiet_NaN(), "unsupported", "No limit found with direct substitution or one L'Hopital step."};
}

LimitResult limit_at_infinity(const ast::ExpressionPtr& expression, const std::string& variable, bool positive) {
    auto coefficients = polynomial(expression, variable);
    if (!coefficients) return {false, false, std::numeric_limits<double>::quiet_NaN(), "unsupported", "Only polynomial limits at infinity are currently supported."};
    trim(*coefficients);
    const double leading = coefficients->back();
    if (coefficients->size() == 1) return {true, false, leading, "constant limit", ""};
    const bool diverges_positive = (leading > 0) == positive;
    return {true, true, diverges_positive ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity(), "polynomial growth", "The polynomial diverges in the requested direction."};
}

}  // namespace calcx::solver

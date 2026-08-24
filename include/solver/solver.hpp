#pragma once

#include "ast/expression.hpp"

#include <complex>
#include <string>
#include <vector>

namespace calcx::solver {

struct EquationResult {
    bool supported;
    std::string method;
    std::vector<std::complex<double>> solutions;
    std::string message;
};

EquationResult solve_equation(const ast::ExpressionPtr& left, const ast::ExpressionPtr& right,
                              const std::string& variable);

struct LimitResult {
    bool supported;
    bool infinite;
    double value;
    std::string method;
    std::string message;
};

LimitResult limit(const ast::ExpressionPtr& expression, const std::string& variable, double point);
LimitResult limit_at_infinity(const ast::ExpressionPtr& expression, const std::string& variable, bool positive);

}  // namespace calcx::solver

#pragma once

#include "ast/expression.hpp"

#include <string>
#include <vector>

namespace calcx::calculus {

struct DerivativeStep {
    std::string rule;
    std::string explanation;
    ast::ExpressionPtr result;
};

struct DerivativeResult {
    ast::ExpressionPtr expression;
    std::vector<DerivativeStep> steps;
};

DerivativeResult differentiate(const ast::ExpressionPtr& expression, const std::string& variable);
ast::ExpressionPtr simplify(const ast::ExpressionPtr& expression);

}  // namespace calcx::calculus

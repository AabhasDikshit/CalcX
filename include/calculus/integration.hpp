#pragma once

#include "ast/expression.hpp"

#include <string>

namespace calcx::calculus {

struct IntegrationResult {
    ast::ExpressionPtr antiderivative;
    bool supported;
    std::string message;
};

IntegrationResult integrate(const ast::ExpressionPtr& expression, const std::string& variable);

}  // namespace calcx::calculus

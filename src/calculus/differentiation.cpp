#include "calculus/differentiation.hpp"

#include <cmath>
#include <sstream>

namespace calcx::calculus {
namespace {

using ast::BinaryOperator;
using ast::Expression;
using ast::ExpressionPtr;
using ast::Function;
using ast::UnaryOperator;

ExpressionPtr n(double value) { return Expression::number(value); }
ExpressionPtr add(ExpressionPtr left, ExpressionPtr right) { return Expression::binary(BinaryOperator::Add, std::move(left), std::move(right)); }
ExpressionPtr sub(ExpressionPtr left, ExpressionPtr right) { return Expression::binary(BinaryOperator::Subtract, std::move(left), std::move(right)); }
ExpressionPtr mul(ExpressionPtr left, ExpressionPtr right) { return Expression::binary(BinaryOperator::Multiply, std::move(left), std::move(right)); }
ExpressionPtr div(ExpressionPtr left, ExpressionPtr right) { return Expression::binary(BinaryOperator::Divide, std::move(left), std::move(right)); }
ExpressionPtr pow(ExpressionPtr left, ExpressionPtr right) { return Expression::binary(BinaryOperator::Power, std::move(left), std::move(right)); }
ExpressionPtr fn(Function function, ExpressionPtr argument) { return Expression::function(function, std::move(argument)); }

bool is_number(const ExpressionPtr& expression, double value) {
    return expression->kind() == Expression::Kind::Number && expression->number_value() == value;
}

ExpressionPtr simplify_node(const ExpressionPtr& expression) {
    if (expression->kind() == Expression::Kind::Number || expression->kind() == Expression::Kind::Variable) return expression;
    if (expression->kind() == Expression::Kind::Unary) {
        auto operand = simplify_node(expression->operand());
        if (expression->unary_operator() == UnaryOperator::Positive) return operand;
        if (operand->kind() == Expression::Kind::Number) return n(-operand->number_value());
        return Expression::unary(UnaryOperator::Negate, operand);
    }
    if (expression->kind() == Expression::Kind::Function) return fn(expression->function(), simplify_node(expression->operand()));

    auto left = simplify_node(expression->left());
    auto right = simplify_node(expression->right());
    const auto op = expression->binary_operator();
    if (left->kind() == Expression::Kind::Number && right->kind() == Expression::Kind::Number) {
        return n(Expression::binary(op, left, right)->evaluate());
    }
    if (op == BinaryOperator::Add) {
        if (is_number(left, 0)) return right;
        if (is_number(right, 0)) return left;
        if (left->to_string() == right->to_string()) return mul(n(2), left);
        if (left->kind() == Expression::Kind::Binary && right->kind() == Expression::Kind::Binary &&
            left->binary_operator() == BinaryOperator::Power && right->binary_operator() == BinaryOperator::Power &&
            is_number(left->right(), 2) && is_number(right->right(), 2) &&
            left->left()->kind() == Expression::Kind::Function && right->left()->kind() == Expression::Kind::Function &&
            ((left->left()->function() == Function::Sin && right->left()->function() == Function::Cos) ||
             (left->left()->function() == Function::Cos && right->left()->function() == Function::Sin)) &&
            left->left()->operand()->to_string() == right->left()->operand()->to_string()) return n(1);
    }
    if (op == BinaryOperator::Subtract) {
        if (is_number(right, 0)) return left;
        if (left->to_string() == right->to_string()) return n(0);
    }
    if (op == BinaryOperator::Multiply) {
        if (is_number(left, 0) || is_number(right, 0)) return n(0);
        if (is_number(left, 1)) return right;
        if (is_number(right, 1)) return left;
        if (left->to_string() == right->to_string()) return pow(left, n(2));
    }
    if (op == BinaryOperator::Divide) {
        if (is_number(left, 0)) return n(0);
        if (is_number(right, 1)) return left;
    }
    if (op == BinaryOperator::Power) {
        if (is_number(right, 0)) return n(1);
        if (is_number(right, 1)) return left;
    }
    return Expression::binary(op, left, right);
}

ExpressionPtr derivative_node(const ExpressionPtr& expression, const std::string& variable, std::vector<DerivativeStep>& steps) {
    switch (expression->kind()) {
    case Expression::Kind::Number:
        steps.push_back({"constant rule", "The derivative of a constant is zero.", n(0)});
        return n(0);
    case Expression::Kind::Variable:
        if (expression->variable_name() == variable) {
            steps.push_back({"variable rule", "The derivative of " + variable + " with respect to itself is one.", n(1)});
            return n(1);
        }
        return n(0);
    case Expression::Kind::Unary: {
        auto result = expression->unary_operator() == UnaryOperator::Negate ?
            Expression::unary(UnaryOperator::Negate, derivative_node(expression->operand(), variable, steps)) :
            derivative_node(expression->operand(), variable, steps);
        return result;
    }
    case Expression::Kind::Binary: {
        const auto op = expression->binary_operator();
        auto left = expression->left();
        auto right = expression->right();
        auto dl = derivative_node(left, variable, steps);
        auto dr = derivative_node(right, variable, steps);
        if (op == BinaryOperator::Add || op == BinaryOperator::Subtract) {
            steps.push_back({"sum rule", "Differentiate each term and preserve the operation.", Expression::binary(op, dl, dr)});
            return Expression::binary(op, dl, dr);
        }
        if (op == BinaryOperator::Multiply) {
            auto result = add(mul(dl, right), mul(left, dr));
            steps.push_back({"product rule", "(f g)' = f'g + fg'.", result});
            return result;
        }
        if (op == BinaryOperator::Divide) {
            auto result = div(sub(mul(dl, right), mul(left, dr)), pow(right, n(2)));
            steps.push_back({"quotient rule", "(f/g)' = (f'g - fg')/g^2.", result});
            return result;
        }
        if (op == BinaryOperator::Power && right->kind() == Expression::Kind::Number) {
            auto exponent = right->number_value();
            auto result = mul(mul(n(exponent), pow(left, n(exponent - 1))), dl);
            steps.push_back({"power rule", "Apply the power rule and then the inner derivative.", result});
            return result;
        }
        if (op == BinaryOperator::Power) {
            auto result = mul(pow(left, right), add(mul(dr, fn(Function::Ln, left)), mul(right, div(dl, left))));
            steps.push_back({"general power rule", "Differentiate u^v using logarithmic differentiation.", result});
            return result;
        }
        return n(0);
    }
    case Expression::Kind::Function: {
        auto inner = expression->operand();
        auto inner_derivative = derivative_node(inner, variable, steps);
        ExpressionPtr outer;
        switch (expression->function()) {
        case Function::Sin: outer = fn(Function::Cos, inner); break;
        case Function::Cos: outer = Expression::unary(UnaryOperator::Negate, fn(Function::Sin, inner)); break;
        case Function::Tan: outer = div(n(1), pow(fn(Function::Cos, inner), n(2))); break;
        case Function::Ln: outer = div(n(1), inner); break;
        case Function::Exp: outer = fn(Function::Exp, inner); break;
        case Function::Sqrt: outer = div(n(1), mul(n(2), fn(Function::Sqrt, inner))); break;
        case Function::Asin: outer = div(n(1), fn(Function::Sqrt, sub(n(1), pow(inner, n(2))))); break;
        case Function::Acos: outer = Expression::unary(UnaryOperator::Negate, div(n(1), fn(Function::Sqrt, sub(n(1), pow(inner, n(2)))))); break;
        case Function::Atan: outer = div(n(1), add(n(1), pow(inner, n(2)))); break;
        default: outer = n(0); break;
        }
        auto result = mul(outer, inner_derivative);
        steps.push_back({"chain rule", "Differentiate the outer function and multiply by the inner derivative.", result});
        return result;
    }
    }
    return n(0);
}

}  // namespace

ast::ExpressionPtr simplify(const ast::ExpressionPtr& expression) { return simplify_node(expression); }

DerivativeResult differentiate(const ast::ExpressionPtr& expression, const std::string& variable) {
    std::vector<DerivativeStep> steps;
    auto result = simplify_node(derivative_node(expression, variable, steps));
    return {result, std::move(steps)};
}

}  // namespace calcx::calculus

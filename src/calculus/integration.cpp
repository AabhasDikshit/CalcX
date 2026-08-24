#include "calculus/integration.hpp"

#include "calculus/differentiation.hpp"

#include <cmath>

namespace calcx::calculus {
namespace {
using ast::BinaryOperator;
using ast::Expression;
using ast::ExpressionPtr;
using ast::Function;

ExpressionPtr n(double value) { return Expression::number(value); }
ExpressionPtr mul(ExpressionPtr left, ExpressionPtr right) { return Expression::binary(BinaryOperator::Multiply, std::move(left), std::move(right)); }
ExpressionPtr div(ExpressionPtr left, ExpressionPtr right) { return Expression::binary(BinaryOperator::Divide, std::move(left), std::move(right)); }
ExpressionPtr pow(ExpressionPtr left, ExpressionPtr right) { return Expression::binary(BinaryOperator::Power, std::move(left), std::move(right)); }
ExpressionPtr fn(Function function, ExpressionPtr argument) { return Expression::function(function, std::move(argument)); }

IntegrationResult unsupported() { return {nullptr, false, "No symbolic antiderivative found."}; }
IntegrationResult integrate_node(const ExpressionPtr& expression, const std::string& variable) {
    if (expression->kind() == Expression::Kind::Number) return {mul(expression, Expression::variable(variable)), true, "Constant rule"};
    if (expression->kind() == Expression::Kind::Variable) {
        if (expression->variable_name() != variable) return {mul(expression, Expression::variable(variable)), true, "Constant rule"};
        return {div(pow(expression, n(2)), n(2)), true, "Power rule"};
    }
    if (expression->kind() == Expression::Kind::Unary) {
        auto inner = integrate_node(expression->operand(), variable);
        if (!inner.supported) return inner;
        return {Expression::unary(expression->unary_operator(), inner.antiderivative), true, "Constant multiple rule"};
    }
    if (expression->kind() == Expression::Kind::Binary) {
        auto left = integrate_node(expression->left(), variable);
        auto right = integrate_node(expression->right(), variable);
        if (expression->binary_operator() == BinaryOperator::Add || expression->binary_operator() == BinaryOperator::Subtract) {
            if (!left.supported || !right.supported) return unsupported();
            return {Expression::binary(expression->binary_operator(), left.antiderivative, right.antiderivative), true, "Sum rule"};
        }
        if (expression->binary_operator() == BinaryOperator::Power && expression->left()->kind() == Expression::Kind::Variable &&
            expression->left()->variable_name() == variable && expression->right()->kind() == Expression::Kind::Number) {
            const double exponent = expression->right()->number_value();
            if (exponent == -1) return {fn(Function::Ln, Expression::variable(variable)), true, "Logarithmic rule"};
            return {div(pow(Expression::variable(variable), n(exponent + 1)), n(exponent + 1)), true, "Power rule"};
        }
    }
    if (expression->kind() == Expression::Kind::Function) {
        auto argument = expression->operand();
        if (argument->kind() != Expression::Kind::Variable || argument->variable_name() != variable) return unsupported();
        switch (expression->function()) {
        case Function::Sin: return {Expression::unary(ast::UnaryOperator::Negate, fn(Function::Cos, argument)), true, "Trigonometric rule"};
        case Function::Cos: return {fn(Function::Sin, argument), true, "Trigonometric rule"};
        case Function::Exp: return {fn(Function::Exp, argument), true, "Exponential rule"};
        case Function::Ln: return {Expression::binary(BinaryOperator::Subtract, mul(argument, fn(Function::Ln, argument)), argument), true, "Integration by parts"};
        default: return unsupported();
        }
    }
    return unsupported();
}
}  // namespace

IntegrationResult integrate(const ast::ExpressionPtr& expression, const std::string& variable) {
    auto result = integrate_node(expression, variable);
    if (result.supported) result.antiderivative = simplify(result.antiderivative);
    return result;
}

}  // namespace calcx::calculus

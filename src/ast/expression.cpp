#include "ast/expression.hpp"

#include <iomanip>
#include <limits>
#include <sstream>

namespace calcx::ast {
namespace {

ExpressionPtr make(Expression::Kind kind) {
    return std::shared_ptr<Expression>(new Expression(kind));
}

std::string number_string(double value) {
    if (value == std::floor(value) && std::abs(value) < 1e15) {
        return std::to_string(static_cast<long long>(value));
    }
    std::ostringstream output;
    output << std::setprecision(15) << value;
    return output.str();
}

double function_value(Function function, double value) {
    switch (function) {
    case Function::Abs: return std::abs(value);
    case Function::Sqrt: if (value < 0) throw EvaluationError("sqrt(x) requires x >= 0."); return std::sqrt(value);
    case Function::Cbrt: return std::cbrt(value);
    case Function::Exp: return std::exp(value);
    case Function::Ln: if (value <= 0) throw EvaluationError("ln(x) requires x > 0."); return std::log(value);
    case Function::Log: if (value <= 0) throw EvaluationError("log(x) requires x > 0."); return std::log10(value);
    case Function::Sin: return std::sin(value);
    case Function::Cos: return std::cos(value);
    case Function::Tan: return std::tan(value);
    case Function::Cot: return 1.0 / std::tan(value);
    case Function::Sec: return 1.0 / std::cos(value);
    case Function::Csc: return 1.0 / std::sin(value);
    case Function::Asin: if (value < -1 || value > 1) throw EvaluationError("asin(x) requires -1 <= x <= 1."); return std::asin(value);
    case Function::Acos: if (value < -1 || value > 1) throw EvaluationError("acos(x) requires -1 <= x <= 1."); return std::acos(value);
    case Function::Atan: return std::atan(value);
    case Function::Sinh: return std::sinh(value);
    case Function::Cosh: return std::cosh(value);
    case Function::Tanh: return std::tanh(value);
    }
    throw EvaluationError("Unknown function.");
}

}  // namespace

Expression::Expression(Kind kind) : kind_(kind) {}

ExpressionPtr Expression::number(double value) { auto result = make(Kind::Number); const_cast<Expression*>(result.get())->number_value_ = value; return result; }
ExpressionPtr Expression::variable(std::string name) { auto result = make(Kind::Variable); const_cast<Expression*>(result.get())->variable_name_ = std::move(name); return result; }
ExpressionPtr Expression::binary(BinaryOperator op, ExpressionPtr left, ExpressionPtr right) { auto result = make(Kind::Binary); auto* item = const_cast<Expression*>(result.get()); item->binary_operator_ = op; item->left_ = std::move(left); item->right_ = std::move(right); return result; }
ExpressionPtr Expression::unary(UnaryOperator op, ExpressionPtr operand) { auto result = make(Kind::Unary); auto* item = const_cast<Expression*>(result.get()); item->unary_operator_ = op; item->operand_ = std::move(operand); return result; }
ExpressionPtr Expression::function(Function function, ExpressionPtr argument) { auto result = make(Kind::Function); auto* item = const_cast<Expression*>(result.get()); item->function_ = function; item->operand_ = std::move(argument); return result; }

Expression::Kind Expression::kind() const noexcept { return kind_; }
double Expression::number_value() const { return number_value_; }
const std::string& Expression::variable_name() const { return variable_name_; }
BinaryOperator Expression::binary_operator() const { return binary_operator_; }
UnaryOperator Expression::unary_operator() const { return unary_operator_; }
Function Expression::function() const { return function_; }
const ExpressionPtr& Expression::left() const { return left_; }
const ExpressionPtr& Expression::right() const { return right_; }
const ExpressionPtr& Expression::operand() const { return operand_; }

double Expression::evaluate(const std::unordered_map<std::string, double>& variables) const {
    switch (kind_) {
    case Kind::Number: return number_value_;
    case Kind::Variable: {
        if (variable_name_ == "pi") return 3.14159265358979323846;
        if (variable_name_ == "e") return 2.71828182845904523536;
        const auto found = variables.find(variable_name_);
        if (found == variables.end()) throw EvaluationError("Unknown variable: " + variable_name_);
        return found->second;
    }
    case Kind::Unary: return unary_operator_ == UnaryOperator::Negate ? -operand_->evaluate(variables) : operand_->evaluate(variables);
    case Kind::Function: return function_value(function_, operand_->evaluate(variables));
    case Kind::Binary: {
        const double left = left_->evaluate(variables);
        const double right = right_->evaluate(variables);
        switch (binary_operator_) {
        case BinaryOperator::Add: return left + right;
        case BinaryOperator::Subtract: return left - right;
        case BinaryOperator::Multiply: return left * right;
        case BinaryOperator::Divide: if (right == 0) throw EvaluationError("Division by zero."); return left / right;
        case BinaryOperator::Power: return std::pow(left, right);
        case BinaryOperator::Modulo: if (right == 0) throw EvaluationError("Modulo by zero."); return std::fmod(left, right);
        }
    }
    }
    throw EvaluationError("Invalid expression.");
}

const char* function_name(Function function) noexcept {
    switch (function) {
    case Function::Abs: return "abs"; case Function::Sqrt: return "sqrt"; case Function::Cbrt: return "cbrt"; case Function::Exp: return "exp"; case Function::Ln: return "ln"; case Function::Log: return "log"; case Function::Sin: return "sin"; case Function::Cos: return "cos"; case Function::Tan: return "tan"; case Function::Cot: return "cot"; case Function::Sec: return "sec"; case Function::Csc: return "csc"; case Function::Asin: return "asin"; case Function::Acos: return "acos"; case Function::Atan: return "atan"; case Function::Sinh: return "sinh"; case Function::Cosh: return "cosh"; case Function::Tanh: return "tanh";
    }
    return "unknown";
}

const char* binary_operator_symbol(BinaryOperator op) noexcept {
    switch (op) { case BinaryOperator::Add: return "+"; case BinaryOperator::Subtract: return "-"; case BinaryOperator::Multiply: return "*"; case BinaryOperator::Divide: return "/"; case BinaryOperator::Power: return "^"; case BinaryOperator::Modulo: return "%"; }
    return "?";
}

std::string Expression::to_string() const {
    switch (kind_) {
    case Kind::Number: return number_string(number_value_);
    case Kind::Variable: return variable_name_;
    case Kind::Unary: return unary_operator_ == UnaryOperator::Negate ? "(-" + operand_->to_string() + ")" : operand_->to_string();
    case Kind::Function: return std::string(function_name(function_)) + "(" + operand_->to_string() + ")";
    case Kind::Binary: return "(" + left_->to_string() + binary_operator_symbol(binary_operator_) + right_->to_string() + ")";
    }
    return "?";
}

}  // namespace calcx::ast

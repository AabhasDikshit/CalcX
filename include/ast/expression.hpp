#pragma once

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace calcx::ast {

enum class BinaryOperator { Add, Subtract, Multiply, Divide, Power, Modulo };
enum class UnaryOperator { Negate, Positive };
enum class Function { Abs, Sqrt, Cbrt, Exp, Ln, Log, Sin, Cos, Tan, Cot, Sec, Csc,
                      Asin, Acos, Atan, Sinh, Cosh, Tanh };

class EvaluationError : public std::runtime_error {
public:
    explicit EvaluationError(const std::string& message) : std::runtime_error(message) {}
};

class Expression;
using ExpressionPtr = std::shared_ptr<const Expression>;

class Expression {
public:
    enum class Kind { Number, Variable, Binary, Unary, Function };

    static ExpressionPtr number(double value);
    static ExpressionPtr variable(std::string name);
    static ExpressionPtr binary(BinaryOperator op, ExpressionPtr left, ExpressionPtr right);
    static ExpressionPtr unary(UnaryOperator op, ExpressionPtr operand);
    static ExpressionPtr function(Function function, ExpressionPtr argument);

    explicit Expression(Kind kind);

    Kind kind() const noexcept;
    double number_value() const;
    const std::string& variable_name() const;
    BinaryOperator binary_operator() const;
    UnaryOperator unary_operator() const;
    Function function() const;
    const ExpressionPtr& left() const;
    const ExpressionPtr& right() const;
    const ExpressionPtr& operand() const;

    double evaluate(const std::unordered_map<std::string, double>& variables = {}) const;
    std::string to_string() const;

private:
    Kind kind_;
    double number_value_ = 0.0;
    std::string variable_name_;
    BinaryOperator binary_operator_{};
    UnaryOperator unary_operator_{};
    Function function_{};
    ExpressionPtr left_;
    ExpressionPtr right_;
    ExpressionPtr operand_;
};

const char* function_name(Function function) noexcept;
const char* binary_operator_symbol(BinaryOperator op) noexcept;

}  // namespace calcx::ast

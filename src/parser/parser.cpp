#include "parser/parser.hpp"

#include "core/tokenizer.hpp"

#include <cstdlib>
#include <sstream>
#include <unordered_map>

namespace calcx::parser {
namespace {

using calcx::ast::BinaryOperator;
using calcx::ast::ExpressionPtr;
using calcx::ast::Function;
using calcx::ast::UnaryOperator;
using calcx::core::Token;
using calcx::core::TokenType;

const std::unordered_map<std::string, Function> functions = {
    {"abs", Function::Abs}, {"sqrt", Function::Sqrt}, {"cbrt", Function::Cbrt},
    {"exp", Function::Exp}, {"ln", Function::Ln}, {"log", Function::Log},
    {"sin", Function::Sin}, {"cos", Function::Cos}, {"tan", Function::Tan},
    {"cot", Function::Cot}, {"sec", Function::Sec}, {"csc", Function::Csc},
    {"asin", Function::Asin}, {"acos", Function::Acos}, {"atan", Function::Atan},
    {"sinh", Function::Sinh}, {"cosh", Function::Cosh}, {"tanh", Function::Tanh}
};

class RecursiveParser {
public:
    explicit RecursiveParser(const std::string& source) : tokens_(calcx::core::Tokenizer{}.tokenize(source)) {}

    ExpressionPtr parse() {
        if (peek().type == TokenType::End) throw ParseError(0, "An expression is required.");
        auto result = parse_additive();
        if (peek().type != TokenType::End) fail("Unexpected token '" + peek().lexeme + "'.");
        return result;
    }

private:
    const Token& peek() const { return tokens_[index_]; }
    const Token& advance() { return tokens_[index_++]; }

    bool match(TokenType type) {
        if (peek().type != type) return false;
        ++index_;
        return true;
    }

    [[noreturn]] void fail(const std::string& message) const {
        throw ParseError(peek().position, message);
    }

    ExpressionPtr parse_additive() {
        auto result = parse_multiplicative();
        while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
            const auto op = advance().type == TokenType::Plus ? BinaryOperator::Add : BinaryOperator::Subtract;
            result = ast::Expression::binary(op, result, parse_multiplicative());
        }
        return result;
    }

    bool starts_implicit_factor() const {
        return peek().type == TokenType::Number || peek().type == TokenType::Identifier ||
               peek().type == TokenType::LeftParenthesis;
    }

    ExpressionPtr parse_multiplicative() {
        auto result = parse_unary();
        while (true) {
            BinaryOperator op;
            if (match(TokenType::Star)) op = BinaryOperator::Multiply;
            else if (match(TokenType::Slash)) op = BinaryOperator::Divide;
            else if (match(TokenType::Percent)) op = BinaryOperator::Modulo;
            else if (starts_implicit_factor()) op = BinaryOperator::Multiply;
            else break;
            result = ast::Expression::binary(op, result, parse_unary());
        }
        return result;
    }

    ExpressionPtr parse_unary() {
        if (match(TokenType::Minus)) return ast::Expression::unary(UnaryOperator::Negate, parse_unary());
        if (match(TokenType::Plus)) return ast::Expression::unary(UnaryOperator::Positive, parse_unary());
        return parse_power();
    }

    ExpressionPtr parse_power() {
        auto result = parse_primary();
        if (match(TokenType::Caret)) result = ast::Expression::binary(BinaryOperator::Power, result, parse_unary());
        return result;
    }

    ExpressionPtr parse_primary() {
        if (peek().type == TokenType::Number) {
            const auto token = advance();
            return ast::Expression::number(std::strtod(token.lexeme.c_str(), nullptr));
        }
        if (peek().type == TokenType::Identifier) {
            const auto token = advance();
            const auto found = functions.find(token.lexeme);
            if (found == functions.end()) return ast::Expression::variable(token.lexeme);
            if (!match(TokenType::LeftParenthesis)) fail("Function '" + token.lexeme + "' requires parentheses.");
            auto argument = parse_additive();
            if (!match(TokenType::RightParenthesis)) fail("Expected ')' after function argument.");
            return ast::Expression::function(found->second, argument);
        }
        if (match(TokenType::LeftParenthesis)) {
            auto result = parse_additive();
            if (!match(TokenType::RightParenthesis)) fail("Expected ')'.");
            return result;
        }
        fail("Expected a number, variable, function, or '('.");
    }

    std::vector<Token> tokens_;
    std::size_t index_ = 0;
};

}  // namespace

ParseError::ParseError(std::size_t position, const std::string& message)
    : std::runtime_error(message), position_(position) {}

std::size_t ParseError::position() const noexcept { return position_; }

ast::ExpressionPtr Parser::parse(const std::string& expression) const {
    return RecursiveParser(expression).parse();
}

}  // namespace calcx::parser

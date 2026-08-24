#include "core/tokenizer.hpp"

#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

using calcx::core::Token;
using calcx::core::TokenType;
using calcx::core::TokenizationError;
using calcx::core::Tokenizer;

void expect_types_and_lexemes(const std::string& expression,
                              const std::vector<std::pair<TokenType, std::string>>& expected) {
    const auto tokens = Tokenizer{}.tokenize(expression);
    assert(tokens.size() == expected.size() + 1);
    for (std::size_t index = 0; index < expected.size(); ++index) {
        assert(tokens[index].type == expected[index].first);
        assert(tokens[index].lexeme == expected[index].second);
    }
    assert(tokens.back().type == TokenType::End);
}

void expect_tokenization_error(const std::string& expression, std::size_t position) {
    bool threw = false;
    try {
        Tokenizer{}.tokenize(expression);
    } catch (const TokenizationError& error) {
        threw = true;
        assert(error.position() == position);
    }
    assert(threw);
}

int main() {
    expect_types_and_lexemes("2 + 3*x^2", {
        {TokenType::Number, "2"}, {TokenType::Plus, "+"}, {TokenType::Number, "3"},
        {TokenType::Star, "*"}, {TokenType::Identifier, "x"}, {TokenType::Caret, "^"},
        {TokenType::Number, "2"}
    });
    expect_types_and_lexemes("3x + 2sin(x) + (x+1)(x-1)", {
        {TokenType::Number, "3"}, {TokenType::Identifier, "x"}, {TokenType::Plus, "+"},
        {TokenType::Number, "2"}, {TokenType::Identifier, "sin"}, {TokenType::LeftParenthesis, "("},
        {TokenType::Identifier, "x"}, {TokenType::RightParenthesis, ")"}, {TokenType::Plus, "+"},
        {TokenType::LeftParenthesis, "("}, {TokenType::Identifier, "x"}, {TokenType::Plus, "+"},
        {TokenType::Number, "1"}, {TokenType::RightParenthesis, ")"}, {TokenType::LeftParenthesis, "("},
        {TokenType::Identifier, "x"}, {TokenType::Minus, "-"}, {TokenType::Number, "1"},
        {TokenType::RightParenthesis, ")"}
    });
    expect_types_and_lexemes(".5 1.0e-3 pi _temp", {
        {TokenType::Number, ".5"}, {TokenType::Number, "1.0e-3"}, {TokenType::Identifier, "pi"},
        {TokenType::Identifier, "_temp"}
    });
    expect_types_and_lexemes("-x, 50%2", {
        {TokenType::Minus, "-"}, {TokenType::Identifier, "x"}, {TokenType::Comma, ","},
        {TokenType::Number, "50"}, {TokenType::Percent, "%"}, {TokenType::Number, "2"}
    });

    expect_tokenization_error("1e+", 0);
    expect_tokenization_error(".", 0);
    expect_tokenization_error("x @ 2", 2);

    std::cout << "Tokenizer tests passed\n";
}

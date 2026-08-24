#include "core/tokenizer.hpp"

#include <cctype>
#include <sstream>

namespace calcx::core {
namespace {

bool is_identifier_start(char character) {
    return std::isalpha(static_cast<unsigned char>(character)) != 0 || character == '_';
}

bool is_identifier_character(char character) {
    return std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '_';
}

bool is_digit(char character) {
    return std::isdigit(static_cast<unsigned char>(character)) != 0;
}

}  // namespace

const char* token_type_name(TokenType type) noexcept {
    switch (type) {
    case TokenType::Number: return "number";
    case TokenType::Identifier: return "identifier";
    case TokenType::Plus: return "+";
    case TokenType::Minus: return "-";
    case TokenType::Star: return "*";
    case TokenType::Slash: return "/";
    case TokenType::Caret: return "^";
    case TokenType::Percent: return "%";
    case TokenType::LeftParenthesis: return "(";
    case TokenType::RightParenthesis: return ")";
    case TokenType::Comma: return ",";
    case TokenType::End: return "end of input";
    }
    return "unknown";
}

TokenizationError::TokenizationError(std::size_t position, const std::string& message)
    : std::runtime_error(message), position_(position) {}

std::size_t TokenizationError::position() const noexcept {
    return position_;
}

std::vector<Token> Tokenizer::tokenize(const std::string& expression) const {
    std::vector<Token> tokens;
    std::size_t position = 0;

    while (position < expression.size()) {
        const char character = expression[position];

        if (std::isspace(static_cast<unsigned char>(character)) != 0) {
            ++position;
            continue;
        }

        if (is_digit(character) || character == '.') {
            const std::size_t start = position;
            bool has_digits = false;

            while (position < expression.size() && is_digit(expression[position])) {
                has_digits = true;
                ++position;
            }
            if (position < expression.size() && expression[position] == '.') {
                ++position;
                while (position < expression.size() && is_digit(expression[position])) {
                    has_digits = true;
                    ++position;
                }
            }
            if (!has_digits) {
                throw TokenizationError(start, "A decimal point must be followed by digits.");
            }
            if (position < expression.size() && (expression[position] == 'e' || expression[position] == 'E')) {
                ++position;
                if (position < expression.size() && (expression[position] == '+' || expression[position] == '-')) {
                    ++position;
                }
                const std::size_t exponent_start = position;
                while (position < expression.size() && is_digit(expression[position])) {
                    ++position;
                }
                if (position == exponent_start) {
                    throw TokenizationError(start, "An exponent must contain digits.");
                }
            }

            tokens.push_back({TokenType::Number, expression.substr(start, position - start), start});
            continue;
        }

        if (is_identifier_start(character)) {
            const std::size_t start = position++;
            while (position < expression.size() && is_identifier_character(expression[position])) {
                ++position;
            }
            tokens.push_back({TokenType::Identifier, expression.substr(start, position - start), start});
            continue;
        }

        TokenType type;
        switch (character) {
        case '+': type = TokenType::Plus; break;
        case '-': type = TokenType::Minus; break;
        case '*': type = TokenType::Star; break;
        case '/': type = TokenType::Slash; break;
        case '^': type = TokenType::Caret; break;
        case '%': type = TokenType::Percent; break;
        case '(': type = TokenType::LeftParenthesis; break;
        case ')': type = TokenType::RightParenthesis; break;
        case ',': type = TokenType::Comma; break;
        default: {
            std::ostringstream message;
            message << "Unexpected character '" << character << "'.";
            throw TokenizationError(position, message.str());
        }
        }
        tokens.push_back({type, std::string(1, character), position});
        ++position;
    }

    tokens.push_back({TokenType::End, "", expression.size()});
    return tokens;
}

}  // namespace calcx::core

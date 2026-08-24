#pragma once

#include <cstddef>
#include <string>

namespace calcx::core {

enum class TokenType {
    Number,
    Identifier,
    Plus,
    Minus,
    Star,
    Slash,
    Caret,
    Percent,
    LeftParenthesis,
    RightParenthesis,
    Comma,
    End
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::size_t position;
};

const char* token_type_name(TokenType type) noexcept;

}  // namespace calcx::core

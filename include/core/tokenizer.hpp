#pragma once

#include "core/token.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace calcx::core {

class TokenizationError : public std::runtime_error {
public:
    TokenizationError(std::size_t position, const std::string& message);

    std::size_t position() const noexcept;

private:
    std::size_t position_;
};

class Tokenizer {
public:
    std::vector<Token> tokenize(const std::string& expression) const;
};

}  // namespace calcx::core

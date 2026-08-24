#pragma once

#include "ast/expression.hpp"

#include <stdexcept>
#include <string>

namespace calcx::parser {

class ParseError : public std::runtime_error {
public:
    ParseError(std::size_t position, const std::string& message);
    std::size_t position() const noexcept;

private:
    std::size_t position_;
};

class Parser {
public:
    ast::ExpressionPtr parse(const std::string& expression) const;
};

}  // namespace calcx::parser

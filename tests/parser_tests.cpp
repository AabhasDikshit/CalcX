#include "parser/parser.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <unordered_map>

using calcx::ast::EvaluationError;
using calcx::parser::ParseError;
using calcx::parser::Parser;

void expect_value(const std::string& expression, double expected,
                  const std::unordered_map<std::string, double>& variables = {}) {
    const double actual = Parser{}.parse(expression)->evaluate(variables);
    assert(std::abs(actual - expected) < 1e-10);
}

int main() {
    expect_value("2 + 3*x^2", 14.0, {{"x", 2.0}});
    expect_value("3x + 2sin(x)", 3.0 + 2.0 * std::sin(1.0), {{"x", 1.0}});
    expect_value("(x+1)(x-1)", 8.0, {{"x", 3.0}});
    expect_value("sin(pi/2)^2 + cos(0)^2", 2.0);
    expect_value("-2^2", -4.0);
    assert(Parser{}.parse("2 + 3*x^2")->to_string() == "(2+(3*(x^2)))");

    bool parse_failed = false;
    try { Parser{}.parse("sin(x"); } catch (const ParseError&) { parse_failed = true; }
    assert(parse_failed);

    bool domain_failed = false;
    try { Parser{}.parse("ln(x)")->evaluate({{"x", -1.0}}); } catch (const EvaluationError&) { domain_failed = true; }
    assert(domain_failed);

    std::cout << "Parser tests passed\n";
}

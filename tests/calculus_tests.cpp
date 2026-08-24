#include "calculus/differentiation.hpp"
#include "parser/parser.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const auto parse = calcx::parser::Parser{};
    const auto power = calcx::calculus::differentiate(parse.parse("x^5"), "x");
    assert(calcx::calculus::simplify(power.expression)->to_string() == "(5*(x^4))");
    assert(std::abs(power.expression->evaluate({{"x", 2.0}}) - 80.0) < 1e-10);

    const auto chain = calcx::calculus::differentiate(parse.parse("sin(x^2)"), "x");
    assert(chain.steps.size() >= 2);
    assert(std::abs(chain.expression->evaluate({{"x", 1.0}}) - 2.0 * std::cos(1.0)) < 1e-10);

    const auto identity = calcx::calculus::simplify(parse.parse("sin(x)^2 + cos(x)^2"));
    assert(identity->to_string() == "1");

    std::cout << "Calculus tests passed\n";
}

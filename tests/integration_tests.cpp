#include "calculus/integration.hpp"
#include "parser/parser.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const auto parser = calcx::parser::Parser{};
    const auto polynomial = calcx::calculus::integrate(parser.parse("x^2"), "x");
    assert(polynomial.supported);
    assert(std::abs(polynomial.antiderivative->evaluate({{"x", 3.0}}) - 9.0) < 1e-10);

    const auto sine = calcx::calculus::integrate(parser.parse("sin(x)"), "x");
    assert(sine.supported);
    assert(std::abs(sine.antiderivative->evaluate({{"x", 0.0}}) + 1.0) < 1e-10);

    const auto unsupported = calcx::calculus::integrate(parser.parse("sin(x^2)"), "x");
    assert(!unsupported.supported);
    assert(unsupported.message == "No symbolic antiderivative found.");

    std::cout << "Integration tests passed\n";
}

#include "numerical/methods.hpp"
#include "parser/parser.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const auto expression = calcx::parser::Parser{}.parse("sin(x)");
    const auto integral = calcx::numerical::simpson(expression, "x", 0.0, 3.14159265358979323846, 1000);
    assert(std::abs(integral.value - 2.0) < 1e-8);

    const auto slope = calcx::numerical::derivative(calcx::parser::Parser{}.parse("x^2"), "x", 3.0);
    assert(std::abs(slope.value - 6.0) < 1e-5);

    const auto root = calcx::numerical::newton_raphson([](double x) { return x * x - 2.0; },
        [](double x) { return 2.0 * x; }, 1.0);
    assert(std::abs(root.value - std::sqrt(2.0)) < 1e-8);

    const auto bisected = calcx::numerical::bisection([](double x) { return x * x - 2.0; }, 0.0, 2.0);
    assert(std::abs(bisected.value - std::sqrt(2.0)) < 1e-8);

    std::cout << "Numerical tests passed\n";
}

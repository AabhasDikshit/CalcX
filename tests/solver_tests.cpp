#include "parser/parser.hpp"
#include "solver/solver.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const auto parser = calcx::parser::Parser{};
    const auto linear = calcx::solver::solve_equation(parser.parse("2*x+5"), parser.parse("15"), "x");
    assert(linear.supported && linear.solutions.size() == 1);
    assert(std::abs(linear.solutions[0].real() - 5.0) < 1e-10);

    const auto quadratic = calcx::solver::solve_equation(parser.parse("x^2-5*x+6"), parser.parse("0"), "x");
    assert(quadratic.supported && quadratic.solutions.size() == 2);
    assert(std::abs(quadratic.solutions[0].real() - 3.0) < 1e-10 || std::abs(quadratic.solutions[1].real() - 3.0) < 1e-10);

    const auto direct_limit = calcx::solver::limit(parser.parse("x^2+1"), "x", 2.0);
    assert(direct_limit.supported && direct_limit.value == 5.0);
    const auto infinity = calcx::solver::limit_at_infinity(parser.parse("x^2+1"), "x", true);
    assert(infinity.supported && infinity.infinite && std::isinf(infinity.value) && infinity.value > 0);

    std::cout << "Solver tests passed\n";
}

#include "calculus/differentiation.hpp"
#include "parser/parser.hpp"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

void print_usage() {
    std::cout << "Usage:\n"
              << "  calcx <expression>\n"
              << "  calcx --differentiate <expression> [variable]\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return EXIT_FAILURE;
    }

    try {
        calcx::parser::Parser parser;
        if (std::string(argv[1]) == "--differentiate") {
            if (argc < 3 || argc > 4) {
                print_usage();
                return EXIT_FAILURE;
            }
            const std::string variable = argc == 4 ? argv[3] : "x";
            const auto result = calcx::calculus::differentiate(parser.parse(argv[2]), variable);
            std::cout << "Derivative: " << result.expression->to_string() << "\n";
            for (std::size_t index = 0; index < result.steps.size(); ++index) {
                std::cout << "Step " << index + 1 << " (" << result.steps[index].rule << "): "
                          << result.steps[index].explanation << "\n";
            }
            return EXIT_SUCCESS;
        }

        const auto expression = parser.parse(argv[1]);
        std::cout << std::setprecision(15) << "Parsed: " << expression->to_string() << "\n"
                  << "Result: " << expression->evaluate() << "\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "CalcX error: " << error.what() << "\n";
        return EXIT_FAILURE;
    }
}

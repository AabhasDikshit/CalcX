#include "calculus/differentiation.hpp"
#include "calculus/integration.hpp"
#include "parser/parser.hpp"
#include "solver/solver.hpp"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifndef WINAPI_FAMILY_PARTITION
#define WINAPI_FAMILY_PARTITION(x) 1
#endif
#ifndef WINAPI_PARTITION_APP
#define WINAPI_PARTITION_APP 1
#endif
#ifndef WINAPI_PARTITION_SYSTEM
#define WINAPI_PARTITION_SYSTEM 1
#endif
#ifndef WINAPI_PARTITION_DESKTOP
#define WINAPI_PARTITION_DESKTOP 1
#endif
#endif

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace {

using json = nlohmann::json;

std::string trim(std::string value) {
    const auto begin = value.find_first_not_of(" \t\n\r");
    if (begin == std::string::npos) return "";
    const auto end = value.find_last_not_of(" \t\n\r");
    return value.substr(begin, end - begin + 1);
}

std::pair<std::string, std::string> split_equation(const std::string& raw) {
    const auto position = raw.find('=');
    if (position == std::string::npos) {
        return {trim(raw), "0"};
    }
    return {trim(raw.substr(0, position)), trim(raw.substr(position + 1))};
}

std::string format_number(double value) {
    if (std::isnan(value)) return "NaN";
    if (!std::isfinite(value)) return value > 0 ? "Infinity" : "-Infinity";

    std::ostringstream stream;
    stream << std::setprecision(15) << value;
    return stream.str();
}

json error_payload(const std::string& message) {
    return json{{"ok", false}, {"error", message}};
}

void send_json(httplib::Response& response, int status, const json& payload) {
    response.status = status;
    response.set_content(payload.dump(), "application/json");
}

json parse_body(const httplib::Request& request) {
    if (request.body.empty()) {
        throw std::runtime_error("Request body is empty.");
    }
    return json::parse(request.body);
}

std::unordered_map<std::string, double> parse_variables(const json& body) {
    std::unordered_map<std::string, double> variables;
    if (!body.contains("variables") || !body["variables"].is_object()) {
        return variables;
    }
    for (const auto& item : body["variables"].items()) {
        variables[item.key()] = item.value().get<double>();
    }
    return variables;
}

std::string variable_or_default(const json& body) {
    if (body.contains("variable") && body["variable"].is_string()) {
        const auto variable = trim(body["variable"].get<std::string>());
        if (!variable.empty()) return variable;
    }
    return "x";
}

}  // namespace

int main() {
    httplib::Server server;
    calcx::parser::Parser parser;

    server.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"},
    });

    server.Options(R"(.*)", [](const httplib::Request&, httplib::Response& response) {
        response.status = 204;
    });

    server.Get("/api/v1/health", [](const httplib::Request&, httplib::Response& response) {
        send_json(response, 200, json{
            {"ok", true},
            {"service", "CalcX API"},
            {"engine", "C++17 calcx_core"},
            {"endpoints", json::array({
                "/api/v1/health",
                "/api/v1/calculate",
                "/api/v1/differentiate",
                "/api/v1/integrate",
                "/api/v1/solve",
            })},
        });
    });

    server.Post("/api/v1/calculate", [&](const httplib::Request& request, httplib::Response& response) {
        try {
            const auto body = parse_body(request);
            const auto expression = parser.parse(body.at("expression").get<std::string>());
            const auto variables = parse_variables(body);
            const double value = expression->evaluate(variables);

            send_json(response, 200, json{
                {"ok", true},
                {"mode", "calculate"},
                {"parsed", expression->to_string()},
                {"result", value},
                {"result_text", format_number(value)},
            });
        } catch (const std::exception& error) {
            send_json(response, 400, error_payload(error.what()));
        }
    });

    server.Post("/api/v1/differentiate", [&](const httplib::Request& request, httplib::Response& response) {
        try {
            const auto body = parse_body(request);
            const auto expression = parser.parse(body.at("expression").get<std::string>());
            const auto variable = variable_or_default(body);
            const auto derivative = calcx::calculus::differentiate(expression, variable);

            json steps = json::array();
            for (const auto& step : derivative.steps) {
                steps.push_back({
                    {"rule", step.rule},
                    {"explanation", step.explanation},
                    {"result", step.result->to_string()},
                });
            }

            send_json(response, 200, json{
                {"ok", true},
                {"mode", "differentiate"},
                {"variable", variable},
                {"derivative", derivative.expression->to_string()},
                {"steps", steps},
            });
        } catch (const std::exception& error) {
            send_json(response, 400, error_payload(error.what()));
        }
    });

    server.Post("/api/v1/integrate", [&](const httplib::Request& request, httplib::Response& response) {
        try {
            const auto body = parse_body(request);
            const auto expression = parser.parse(body.at("expression").get<std::string>());
            const auto variable = variable_or_default(body);
            const auto integration = calcx::calculus::integrate(expression, variable);

            send_json(response, 200, json{
                {"ok", true},
                {"mode", "integrate"},
                {"variable", variable},
                {"supported", integration.supported},
                {"message", integration.message},
                {"antiderivative", integration.supported ? integration.antiderivative->to_string() : ""},
            });
        } catch (const std::exception& error) {
            send_json(response, 400, error_payload(error.what()));
        }
    });

    server.Post("/api/v1/solve", [&](const httplib::Request& request, httplib::Response& response) {
        try {
            const auto body = parse_body(request);
            const auto variable = variable_or_default(body);

            std::string left_text;
            std::string right_text;
            if (body.contains("left") && body.contains("right")) {
                left_text = body.at("left").get<std::string>();
                right_text = body.at("right").get<std::string>();
            } else {
                const auto equation = body.at("equation").get<std::string>();
                const auto split = split_equation(equation);
                left_text = split.first;
                right_text = split.second;
            }

            const auto left = parser.parse(left_text);
            const auto right = parser.parse(right_text);
            const auto solved = calcx::solver::solve_equation(left, right, variable);

            json solutions = json::array();
            for (const auto& value : solved.solutions) {
                solutions.push_back({
                    {"real", value.real()},
                    {"imag", value.imag()},
                    {"text", std::abs(value.imag()) < 1e-12
                        ? format_number(value.real())
                        : format_number(value.real()) + " + " + format_number(value.imag()) + "i"},
                });
            }

            send_json(response, 200, json{
                {"ok", true},
                {"mode", "solve"},
                {"variable", variable},
                {"supported", solved.supported},
                {"method", solved.method},
                {"message", solved.message},
                {"solutions", solutions},
            });
        } catch (const std::exception& error) {
            send_json(response, 400, error_payload(error.what()));
        }
    });

    const int port = std::getenv("CALCX_API_PORT") ? std::atoi(std::getenv("CALCX_API_PORT")) : 8080;
    std::cout << "CalcX API listening on http://0.0.0.0:" << port << "\n";
    std::cout << "Health endpoint: http://127.0.0.1:" << port << "/api/v1/health\n";
    if (!server.listen("0.0.0.0", port)) {
        std::cerr << "Failed to start CalcX API server on port " << port << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
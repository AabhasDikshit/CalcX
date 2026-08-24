cmake_minimum_required(VERSION 3.16)
project(CalcX VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CALCX_BUILD_API "Build CalcX HTTP API adapter" OFF)

add_library(calcx_core
    src/ast/expression.cpp
    src/calculus/differentiation.cpp
    src/calculus/integration.cpp
    src/numerical/methods.cpp
    src/solver/solver.cpp
    src/parser/parser.cpp
    src/core/tokenizer.cpp
)
target_include_directories(calcx_core PUBLIC include)
target_compile_options(calcx_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

add_executable(calcx apps/calcx.cpp)
target_link_libraries(calcx PRIVATE calcx_core)

if(CALCX_BUILD_API)
    include(FetchContent)
    find_package(Threads REQUIRED)

    FetchContent_Declare(
        cpp_httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.16.3
    )

    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.6.1
    )

    set(JSON_BuildTests OFF CACHE INTERNAL "" FORCE)
    FetchContent_MakeAvailable(cpp_httplib nlohmann_json)

    add_executable(calcx_api apps/calcx_api.cpp)
    target_link_libraries(calcx_api PRIVATE calcx_core httplib::httplib nlohmann_json::nlohmann_json Threads::Threads)
    if(WIN32)
        target_link_libraries(calcx_api PRIVATE ws2_32)
    endif()
endif()

add_executable(calcx_tokenizer_tests tests/tokenizer_tests.cpp)
target_link_libraries(calcx_tokenizer_tests PRIVATE calcx_core)

add_executable(calcx_parser_tests tests/parser_tests.cpp)
target_link_libraries(calcx_parser_tests PRIVATE calcx_core)

add_executable(calcx_calculus_tests tests/calculus_tests.cpp)
target_link_libraries(calcx_calculus_tests PRIVATE calcx_core)

add_executable(calcx_numerical_tests tests/numerical_tests.cpp)
target_link_libraries(calcx_numerical_tests PRIVATE calcx_core)

add_executable(calcx_integration_tests tests/integration_tests.cpp)
target_link_libraries(calcx_integration_tests PRIVATE calcx_core)

add_executable(calcx_solver_tests tests/solver_tests.cpp)
target_link_libraries(calcx_solver_tests PRIVATE calcx_core)

enable_testing()
add_test(NAME tokenizer_tests COMMAND calcx_tokenizer_tests)
add_test(NAME parser_tests COMMAND calcx_parser_tests)
add_test(NAME calculus_tests COMMAND calcx_calculus_tests)
add_test(NAME numerical_tests COMMAND calcx_numerical_tests)
add_test(NAME integration_tests COMMAND calcx_integration_tests)
add_test(NAME solver_tests COMMAND calcx_solver_tests)

# CalcX

CalcX is a C++17 symbolic mathematics laboratory with a mobile-first browser shell. The mathematics engine is independent of the UI and currently supports parsing, evaluation, simplification, differentiation, basic integration, numerical methods, polynomial equations, and selected limits.

## Quick Start

Install CMake 3.16 or newer and a C++17 compiler. On Windows, MinGW-w64 or Visual Studio works. From PowerShell at the repository root:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
Push-Location build
ctest --output-on-failure
Pop-Location
```

The executable is `build/calcx.exe` on Windows or `build/calcx` on Unix-like systems.

## Running CalcX

Evaluate a constant expression:

```powershell
.\build\calcx.exe "2+3*4"
```

Differentiate with respect to `x`:

```powershell
.\build\calcx.exe --differentiate "sin(x^2)" x
```

The CLI prints the parsed AST, result, or symbolic derivative steps. Variables require a future command/API binding for values; the library API accepts a variable map directly.

Open `web/index.html` in a browser for the mobile-first interface. The current shell provides keypad entry, operation modes, theme switching, and local history. It clearly labels the pending HTTP bridge rather than evaluating mathematics in JavaScript.

## Features

- Tokenizer with source positions and malformed-input diagnostics
- Recursive-descent parser with precedence and implicit multiplication
- Shared immutable AST for numbers, variables, operators, unary signs, and functions
- Real-valued evaluation with constants `pi` and `e`
- Simplification of arithmetic identities and `sin(x)^2 + cos(x)^2`
- Symbolic power, sum, product, quotient, chain, and general-power differentiation
- Generated derivative rule steps
- Basic symbolic integration with explicit unsupported results
- Trapezoidal and Simpson integration
- Central-difference numerical derivatives
- Newton-Raphson and bisection root finding
- Linear and quadratic polynomial equation solving
- Direct-substitution and selected L'Hopital limit handling
- Responsive browser shell with persistent local history

## Technology Behind It

The engine is standard C++17 and uses only the STL. Input flows through `Tokenizer -> Token stream -> Parser -> AST`. Recursive-descent functions encode precedence: additive, multiplicative, unary, power, and primary expressions. AST nodes use `std::shared_ptr<const Expression>` so calculus passes can share trees without accidental mutation.

Evaluation recursively walks the AST and uses an environment map for variables. Differentiation applies structural rules recursively and records `DerivativeStep` values. Simplification performs bounded local rewrites and constant folding. Polynomial solving extracts coefficient vectors, then applies the linear or quadratic formula. Numerical algorithms use configurable interval, tolerance, and iteration parameters and throw descriptive errors on invalid domains or failed convergence.

No external CAS, SymPy, WolframAlpha, or arbitrary code execution is used. Unsupported symbolic operations return a capability message instead of an invented answer.

## Repository Layout

- `include/`: public C++ interfaces for AST, parser, calculus, numerical methods, and solvers
- `src/`: C++ implementations
- `apps/`: CLI entry point
- `tests/`: executable CTest regression suites
- `web/`: mobile-first browser shell
- `docs/`: parser design notes

## Testing

The test suite covers tokenization, precedence, implicit multiplication, constants, domain errors, derivatives, trig simplification, symbolic integration, numerical integration, root finding, equations, and limits.

```powershell
cmake --build build
Push-Location build
ctest --output-on-failure
Pop-Location
```

## Limitations and Roadmap

The current release does not yet provide a network API, full rational-function limits, trigonometric general-solution formatting, Taylor series, ODEs, multivariable calculus, graphing, or partial-fraction integration. These are planned extensions. Mathematical correctness takes priority over claiming unsupported coverage.

## Publishing on GitHub

Create an empty repository named `CalcX` under your GitHub account, then run:

```powershell
git add .
git commit -m "Build CalcX symbolic mathematics foundation"
git branch -M main
git remote add origin https://github.com/<your-user>/CalcX.git
git push -u origin main
```

Do not commit `build/`, IDE files, or generated binaries; they are covered by `.gitignore`.

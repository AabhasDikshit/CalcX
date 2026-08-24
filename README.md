# CalcX

CalcX is a C++17 symbolic mathematics laboratory with a mobile-first web interface.

The mathematics engine is implemented in C++ (`calcx_core`) and is reused by:

- CLI executable: `calcx`
- HTTP backend executable: `calcx_api`
- Static frontend in `web/` (deployed separately)

## Architecture

GitHub Pages cannot execute C++ binaries, so deployment is intentionally split:

- Frontend (static): GitHub Pages (`web/`)
- Backend (dynamic): hosted process running `calcx_api`
- Math engine: `calcx_core` library linked into `calcx_api`

Data flow:

Phone browser -> GitHub Pages UI -> HTTP request -> `calcx_api` -> `calcx_core` -> result on phone.

## Repository Layout

- `include/` public C++ interfaces
- `src/` C++ implementations
- `apps/calcx.cpp` CLI entry
- `apps/calcx_api.cpp` HTTP API entry
- `tests/` CTest suites
- `web/` mobile-first frontend for GitHub Pages
- `docs/api.md` API contract and examples

## Local Build and Test

Prerequisites:

- CMake 3.16+
- C++17 compiler

Build and test:

```powershell
cmake -S . -B build
cmake --build build --config Release
Push-Location build
ctest --output-on-failure
Pop-Location
```

## Run the C++ API Backend

Start API locally:

```powershell
cmake -S . -B build-api -DCALCX_BUILD_API=ON
cmake --build build-api --config Release
.\\build-api\\calcx_api.exe
```

Default address:

- `http://127.0.0.1:8080`
- Health check: `http://127.0.0.1:8080/api/v1/health`

Port override:

```powershell
$env:CALCX_API_PORT = "8080"
.\build\calcx_api.exe
```

## GitHub Pages Deployment

The workflow file `.github/workflows/deploy-pages.yml` publishes `web/` automatically.

1. Push to `main`.
2. In GitHub repo settings, enable Pages and set source to GitHub Actions.
3. Wait for action `Deploy CalcX Web to Pages` to finish.

Expected Pages URL for this repository:

- `https://aabhasdikshit.github.io/CalcX/`

## Mobile Usage (Android/iPhone)

### 1) Open the public frontend URL

- Open `https://aabhasdikshit.github.io/CalcX/` on your phone.

### 2) Set backend API URL

In the app, set **API Base URL** to your running backend service, for example:

- `https://your-calcx-api.example.com`

Tap **Save**.

You can also prefill it using query string:

- `https://aabhasdikshit.github.io/CalcX/?api=https://your-calcx-api.example.com`

### 3) Use calculation modes

- `Calculate`: numeric evaluation from C++ engine
- `Derivative`: symbolic differentiation from C++ engine
- `Integral`: symbolic integration from C++ engine (with explicit unsupported messages)
- `Solve`: polynomial equation solving (up to degree 2) from C++ engine

## Add to Home Screen

### Android (Chrome)

1. Open the CalcX URL in Chrome.
2. Tap menu (three dots).
3. Tap **Add to Home screen** or **Install app**.
4. Confirm.

### iPhone (Safari)

1. Open the CalcX URL in Safari.
2. Tap **Share**.
3. Tap **Add to Home Screen**.
4. Confirm.

## What Works vs What Requires Backend

Works with only GitHub Pages frontend:

- Mobile UI rendering
- Keypad and expression editing
- Mode switching
- Theme toggle
- Local calculation history in browser storage

Requires running C++ backend (`calcx_api`):

- Actual evaluation results
- Symbolic differentiation
- Symbolic integration
- Equation solving

## CLI Usage

Evaluate:

```powershell
.\build\calcx.exe "2+3*4"
```

Differentiate:

```powershell
.\build\calcx.exe --differentiate "sin(x^2)" x
```

## CI

- `.github/workflows/ci.yml` builds and runs CTest on push/PR.
- `.github/workflows/deploy-pages.yml` deploys `web/` to GitHub Pages.

## Limitations

- GitHub Pages hosts only static frontend assets, not C++ processes.
- The backend must be hosted separately (VM/container/service).
- Equation solver currently supports polynomial equations up to degree 2.

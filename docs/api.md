# CalcX HTTP API

This API keeps the existing C++17 engine (`calcx_core`) as the source of truth. The web frontend is static and can be hosted on GitHub Pages, while the API process must run on a backend host.

## Start the API server

From repo root:

```powershell
cmake -S . -B build-api -DCALCX_BUILD_API=ON
cmake --build build-api --config Release
.\build-api\calcx_api.exe
```

Optional port override:

```powershell
$env:CALCX_API_PORT = "8080"
.\build-api\calcx_api.exe
```

## Endpoints

Base path: `/api/v1`

- `GET /health`
- `POST /calculate`
- `POST /differentiate`
- `POST /integrate`
- `POST /solve`

### Calculate

Request:

```json
{
  "expression": "2+3*4",
  "variables": {"x": 2}
}
```

### Differentiate

Request:

```json
{
  "expression": "sin(x^2)",
  "variable": "x"
}
```

### Integrate

Request:

```json
{
  "expression": "x^2",
  "variable": "x"
}
```

### Solve

Request:

```json
{
  "equation": "x^2-5x+6=0",
  "variable": "x"
}
```

You can also send left/right:

```json
{
  "left": "x^2-5x+6",
  "right": "0",
  "variable": "x"
}
```

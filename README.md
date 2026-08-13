# hif-backend

**hif-backend** provides `hif2verilog` and `hif2vhdl` — code generators that translate the HDL-Independent Format (HIF) back into Verilog and VHDL. A third generator, `hif2sc` (SystemC), is built automatically when a SystemC installation is detected, and skipped otherwise.

Part of the HIF toolchain for HDL-independent-format compilation:
- [hif-core](https://github.com/hif-project/hif-core) — shared AST/IR library
- [hif-frontend](https://github.com/hif-project/hif-frontend) — Verilog/VHDL → HIF
- **hif-backend** (this repo) — HIF → Verilog/VHDL(/SystemC)
- [hif-muffin](https://github.com/hif-project/hif-muffin) — RTL fault injection, built on the above

![CI](https://github.com/hif-project/hif-backend/actions/workflows/ci.yml/badge.svg?branch=develop)

## Requirements

- Linux (only supported/tested platform)
- CMake ≥ 3.1, a C++17 compiler (GCC or Clang)
- A build of [hif-core](https://github.com/hif-project/hif-core)
- (Optional) SystemC — only needed to enable `hif2sc`; if not found, `hif2sc` is silently disabled and `hif2verilog`/`hif2vhdl` build normally

## Building

`hif-backend` links against `hif-core` via `find_package(HIF REQUIRED)` (see `cmake/FindHIF.cmake`). If `hif-core` isn't installed system-wide, either check it out as a sibling directory (`../hif-core`, built with its own `cmake`/`make` — no install needed), or point CMake at it explicitly:

```sh
mkdir build && cd build
cmake -DHIF_DIR=/path/to/hif-core ..
make
```

## Running tests

```sh
ctest --test-dir build --output-on-failure
```

One test, `unresolved_parameter`, needs a sibling `hif-frontend` build (`../hif-frontend/build`) to produce its input HIF; it's skipped if that's not found. It is also a **known, tracked failure** — hif-core's `VerilogAnalysis` currently has no typing rule for two `Int`-typed operands, so this fixture's `hif2verilog` step aborts. The test is marked `WILL_FAIL` so this stays visible without failing CI: if it ever unexpectedly starts passing, that's the signal the underlying gap has been fixed.

## Documentation

If Doxygen is available:

```sh
make backend_documentation
```

## License

BSD 2-Clause. See [LICENSE.md](LICENSE.md).

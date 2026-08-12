# Changelog

All notable changes to this project are documented in this file.

## [Unreleased]

## [1.0.0] - 2026-08-12

Initial coordinated release of the HIF toolchain baseline (hif-core, hif-frontend, hif-backend, hif-muffin, all tagged v1.0.0).

- Fixed `VerilogPrinter` incorrectly using VHDL semantics instead of Verilog semantics.
- Fixed CI to build its `hif-core`/`hif-frontend` dependencies and consolidated it to a single Linux workflow. `unresolved_parameter` is now a tracked, visible regression (`WILL_FAIL`) instead of silently never running.
- Removed a dead unused field in `hif2verilog`'s `VerilogVisitor`, surfaced once CI actually reached a real Clang build for the first time.

### Known limitations

`unresolved_parameter` is an intentional, tracked failure: hif-core's `VerilogAnalysis` has no typing rule for two `Int`-typed operands under Verilog semantics. See hif-muffin's `docs/known-limitations.md` for the full toolchain compatibility reference.

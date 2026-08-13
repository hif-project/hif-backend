# Changelog

All notable changes to this project are documented in this file.

## [Unreleased]

## [1.1.0] - 2026-08-13

- Fixed `VerilogPrinter` to print the reduction-AND/OR/XOR operators.
- Fixed rendering of unresolved parametric port widths in `hif2verilog`.
- Fixed printing of `iterated_concat` (Verilog's `{N{expr}}` replication) and four-state (`x`/`z`) values. `unresolved_parameter` now passes for real — the `Int`-`Int` typing gap noted below under 1.0.0 was fixed upstream in hif-core v1.1.0, and the `WILL_FAIL` tripwire has been removed.
- Fixed `VerilogPrinter` silently dropping `Procedure` declarations (cone functions) when emitting module instances.
- Fixed `VerilogPrinter` printing concatenation as a broken infix operator instead of `{a, b}` syntax.
- Fixed `VerilogPrinter` silently dropping `localparam` declarations.
- Migrated the project to the `hif-project` GitHub organization; updated internal references accordingly.
- Replaced the README's ecosystem-navigation list with a link to the organization profile.

## [1.0.0] - 2026-08-12

Initial coordinated release of the HIF toolchain baseline (hif-core, hif-frontend, hif-backend, hif-muffin, all tagged v1.0.0).

- Fixed `VerilogPrinter` incorrectly using VHDL semantics instead of Verilog semantics.
- Fixed CI to build its `hif-core`/`hif-frontend` dependencies and consolidated it to a single Linux workflow. `unresolved_parameter` is now a tracked, visible regression (`WILL_FAIL`) instead of silently never running.
- Removed a dead unused field in `hif2verilog`'s `VerilogVisitor`, surfaced once CI actually reached a real Clang build for the first time.

### Known limitations

`unresolved_parameter` is an intentional, tracked failure: hif-core's `VerilogAnalysis` has no typing rule for two `Int`-typed operands under Verilog semantics. See hif-muffin's `docs/known-limitations.md` for the full toolchain compatibility reference.

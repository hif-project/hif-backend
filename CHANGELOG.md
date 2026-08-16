# Changelog

All notable changes to this project are documented in this file.

## [Unreleased]

- Fixed `hif2vhdl` failing on every `verilog2hif` output, which made the Verilog → HIF → VHDL direction unavailable for every input rather than for particular constructs. `PreRefine_misc` lowers each `When` into a synthesized `when_function_N` and inserts it into the nearest enclosing `Function` or `Contents`, and it walked the whole description including the standard Verilog library referenced in every `verilog2hif` output. That library declares `hif_verilog__system_readmemb` with a parameter whose default value is a `When`, and a `When` in a `SubProgram` signature has neither an enclosing `Function` nor an enclosing `Contents`, so the pass reached its final `else` and aborted with "Unsupported scope" — exit 1, empty output directory, regardless of what the design contained. Standard `LibraryDef`s are now skipped: they are supplied by the semantics, are never emitted as VHDL, and none of this visitor's rewrites apply to them. This mirrors `hif2sc`'s `WhenSplitter`, which already guarded the same way, so the fix restores a distinction the sibling backend had and this one lacked. Lowering of `When`s in user code is unaffected — the guard keys on `LibraryDef::isStandard()` and by construction cannot reach user code. Verilog → VHDL had apparently never worked; both halves date from their respective initial commits. This unblocks the direction rather than completing it: a conditional assignment is the next thing in the way, because the `If` lowered from a ternary keeps its Verilog bit-typed condition where VHDL requires a boolean, which is tracked separately as #54. (#48)

## [1.2.0] - 2026-08-16

- Fixed `hif2verilog` emitting nothing at all for a Verilog system *task* call. hif-core declares `$display` and its neighbours as subprograms with no return type and no body, so the call resolved to a `Procedure` with no `StateTable` and `visitProcedureCall`'s "nothing to inline" early return dropped it silently: the regenerated design compiled, simulated and printed nothing, discarding the entire observable output of a testbench or an instrumented model. System task calls are now restored to their `$` spelling with their arguments, the task-side counterpart of what #19 did for system functions. Necessarily included: string literals were printed nowhere either (`visitStringValue` wrote nothing), so restoring the call alone produced `$display(, a)` — the format string carrying the message gone, and rejected by `verilog2hif` on the way back in. Literals are written back out verbatim between quotes, since the frontend stores their escapes in source form. The cone-inlining path that `visitProcedureCall` also carries is untouched: the new check precedes it and returns. (#29)
- Fixed `hif2verilog` dropping an output port's initial value, which silently lost a constant continuous assignment: `verilog2hif` folds any constant right-hand side into the driven port's value, and Verilog-2001 has no place for an initializer inside an ANSI port list, so `assign c = 32'd7;` regenerated as an empty module with an undriven output. The value is now re-emitted as the continuous assignment it came from — strictly for ports that nothing else drives, since `vhdl2hif` gives every output port the VHDL `'U'` default whether the source wrote one or not, and emitting one unconditionally would put a second driver on every VHDL output. An output port carrying an initial value while a *process* drives it needs an `initial` block rather than a continuous assign and is deliberately not covered here; it is tracked as #36. (#30)
- Fixed `hif2verilog` never printing a view's `GlobalAction`, which silently dropped every VHDL concurrent signal assignment: a VHDL design regenerated as a module with the correct ports and an empty body, with exit code 0 and output that both compiled and reparsed. Global actions are now emitted as Verilog continuous assignments. Consequently the rule introduced for #26 — a declaration an instance's output drives must be a net — was generalized to cover any *continuous* driver, since a continuous `assign` cannot drive a `reg` either. This is also the first path on which a VHDL `after` delay reaches the output, the machinery for which #24 had already added. Designs arriving through `verilog2hif` are unaffected, as that frontend rewrites continuous assignments into processes. (#32)
- Fixed `hif2vhdl` leaving the design unit it emitted last as a zero-byte file while exiting 0. `IndentedStream` writes from its destructor, and only the *next* unit's setup deleted the previous stream, so the last one was never written — for a single-unit design, that was the whole output. Reported as a hierarchical-parent problem, which is where it happened to be visible. (#27)
- Fixed the delay on a delayed assignment being dropped, so a regenerated design responded immediately where its source waited. The HIF carried it and `visitAssign` never read it. Delays are now emitted as intra-assignment delays, together with the `` `timescale `` directive that establishes the unit they count in. As part of that, the two timescale bookkeeping constants are no longer emitted as valueless `localparam`s — which no simulator accepted, and which made every design carrying an explicit `` `timescale `` regenerate unusable. (#24)
- Fixed a structure-preserving round trip (`verilog2hif -s`) regenerating the parent's connection nets and instance-driven outputs as `reg`, which Verilog forbids for anything an instance drives, so the regenerated hierarchy did not compile at all. Declarations bound to an instance's `out`/`inout` port are now emitted as nets. (#26)
- Fixed Verilog system functions being emitted under their internal name (`hif_verilog__system_clog2` rather than `$clog2`), which no simulator accepts and the frontend cannot bind on the way back in, breaking the round trip of any design using `$clog2`. (#19)

## [1.1.0] - 2026-08-14

- Fixed regenerated Verilog not being behaviorally equivalent to its source. Frontend-generated logic cones were hoisted into their own `always @(*)` blocks and their call dropped, turning an intra-process dependency into an unordered inter-process one, so a process could read a cone target before the block that writes it had run. Cones are now emitted at their call site, inside the process that reads them. (#16)
- Fixed `hif2verilog` aborting on an assignment whose target is a bit-select, which left a zero-byte output file behind. The target's declaration is now resolved through the terminal prefix. (#17)
- Fixed slice emission producing invalid Verilog: a part-select applied to an expression, and part-select bounds printed ahead of their identifier with empty brackets. (#18)
- Fixed module parameters being emitted as body declarations after the port list that references them; they are now emitted as an ANSI `#( ... )` clause. (#20)
- Fixed only one of a process's three sensitivity lists being emitted, with the `posedge`/`negedge` keyword printed once per list rather than once per signal. An asynchronous reset was regenerated as a synchronous one. (#21)
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

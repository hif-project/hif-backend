/// @file PostRefineMethods.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

/// @brief Rebuilds Verilog edge sensitivity for processes that express their
/// clock edge as a condition rather than as a sensitivity qualifier.
///
/// VHDL has no edge-sensitive sensitivity list: a clocked process is sensitive
/// to the clock's level and tests `clk'event and clk = '1'` (or `rising_edge`)
/// inside its body. Verilog has the opposite arrangement, and the edge belongs
/// in the sensitivity list. Without this pass the test is printed as a call to
/// `hif_vhdl_event` / `hif_vhdl_rising_edge`, which is declared nowhere and
/// which no simulator accepts.
///
/// @param o The system object.
/// @param sem The semantics.
void fixSynchronousProcesses(hif::System *o, hif::semantics::ILanguageSemantics *sem);

/// @brief Moves a `For`'s `initDeclarations` out of the loop header.
///
/// HIF lets a `For` declare its own index, and `verilog2hif` uses that when it
/// lowers `repeat (n) ...`. Verilog before SystemVerilog has no C-style
/// declaration inside a `for` header, so the declaration has to live somewhere
/// the printer already emits - the enclosing process's declaration list, which
/// it hoists to the module body - and the index's initial value has to become
/// the loop's init assignment.
///
/// @param o The system object.
/// @param sem The semantics.
void hoistForInitDeclarations(hif::System *o, hif::semantics::ILanguageSemantics *sem);

/// @brief Gives an `inout` port that a process drives a reg to be driven
/// through, plus a continuous assignment from that reg onto the port.
///
/// A port a process drives has to be a reg and one that is continuously driven
/// has to be a wire (hif-backend#26, #32). Verilog-2001 has no `inout reg`, so
/// unlike the `dir_out` case this cannot be settled by choosing a keyword: an
/// `inout` port is always a net, and a net is not a valid procedural l-value.
/// Without this pass the regenerated design does not elaborate, while
/// hif2verilog exits 0 and the output reparses (hif-backend#71).
///
/// The continuous driver is unconditional. VHDL spells the high-impedance
/// state as a *value* of the resolved type, so a process releases the net by
/// assigning 'Z', which reaches Verilog as the ordinary value `1'bz`; a
/// separate enable would have to be derived from the same information and
/// would model nothing the value does not already model.
///
/// @param o The system object.
/// @param sem The semantics.
void lowerProcedurallyDrivenInoutPorts(hif::System *o, hif::semantics::ILanguageSemantics *sem);

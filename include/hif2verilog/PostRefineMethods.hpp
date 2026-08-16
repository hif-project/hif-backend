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

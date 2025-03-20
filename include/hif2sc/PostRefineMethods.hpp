/// @file PostRefineMethods.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

#include "hif2sc/hif2scParseLine.hpp"

/// @brief Optimized tree w.r.t. future generated code.
/// @param o The system object.
/// @param cLine The hif2sc command line options.
/// @param sem The semantics.
///
void postRefinementsOptimizationStep(hif::System *o, hif2scParseLine &cLine, hif::semantics::ILanguageSemantics *sem);

/// @brief Clean the HIF tree before printing.
/// @param o The system object.
/// @param cLine The hif2sc command line options.
/// @param sem The semantics.
///
void postRefinementsFinalStep(hif::System *o, hif2scParseLine &cLine, hif::semantics::ILanguageSemantics *sem);

/// @brief This visitor is intended to grant retro-compatibility of generated code if
/// requested by the user.
/// By default, generated code exploits features of standard ISO/IEC 14882:2011 (C++11).
/// This visitor modifies Hif tree to match features of standard
/// ISO/IEC 14882:1998 (C++98).
/// @param o The system object.
/// @param sem The semantics.
/// @param cLine The hif2sc command line options.
/// @param checkSem The checking semantics.
///
void cpp98StandardRefinements(
    hif::System *o,
    hif2scParseLine &cLine,
    hif::semantics::ILanguageSemantics *sem,
    hif::semantics::ILanguageSemantics *checkSem);

/// @file PostRefineMethods.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

/// @brief Clean the HIF tree before printing.
/// @param o The system object.
/// @param sem The semantics.
///
void postRefinementsFinalStep(hif::System *o, hif::semantics::ILanguageSemantics *sem);

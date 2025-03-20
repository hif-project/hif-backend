/// @file PreRefineMethods.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

#include "hif2sc/hif2scParseLine.hpp"

/// @brief Performs dedicated fixes on particular objects. Fixes contained here
/// must respect the following conditions:
/// - they must not involve other objects except the one that is currently visited;
/// - they must not present temporal reliability on other fixes.
/// @param o The system object.
/// @param sem The semantics.
/// @param checkSem The check semantics.
///
void fixMiscIssues(
    hif::System *o,
    hif::semantics::ILanguageSemantics *sem,
    hif::semantics::ILanguageSemantics *checkSem);

/// @brief Checks if there are subprogram declarations that are in conflict
/// with each other w.r.t. systemC Semantics.
/// For example two conflicts sub declarations are foo( signed a) and
/// foo( unsigned a ) because signed and unsigend are translated both as logic
/// vectors. Also foo() and foo() are conflicting declarations that can appear
/// after other pre refinements.
/// If those declararation are found, they are renamed (renaming also references).
/// @param o The system object.
/// @param keepBit Flat to keep bit type.
/// @param sem The semantics.
/// @return true if found at least one conflicting sub program.
///
bool fixConflictingSubPrograms(hif::System *o, const bool keepBit, hif::semantics::ILanguageSemantics *sem);

/// @brief Add utility library definitions and references.
bool fixUtilityLibraries(hif::System *o, hif::semantics::ILanguageSemantics *sem, const hif2scParseLine &cLine);

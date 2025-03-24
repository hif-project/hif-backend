/// @file PreRefineMethods.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

/// @brief Add utility library definitions and references.
auto fixUtilityLibraries(hif::System *o, hif::semantics::ILanguageSemantics *sem) -> bool;

/// @brief In VHDL the names of identifiers can not contains '__' (double
/// underscore) and have the underscore character as prefix.
void fixIdentifierNames(hif::System *o, hif::semantics::ILanguageSemantics *sem);

/// @brief In HIF an output port can be read. On the contrary, in VHDL they can
/// only be written.
void fixReadOutPorts(hif::System *o, hif::semantics::ILanguageSemantics *sem);

/// @brief Performs dedicated fixes on particular objects. Fixes contained here
/// @param o The system object.
/// @param sem The semantics.
///
void fixMiscIssues(hif::System *o, hif::semantics::ILanguageSemantics *sem);

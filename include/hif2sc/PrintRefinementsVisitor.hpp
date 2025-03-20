/// @file PrintRefinementsVisitor.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

#include "hif2sc/globals.hpp"

void printRefinements(hif::System *sys, hif::semantics::ILanguageSemantics *sem);

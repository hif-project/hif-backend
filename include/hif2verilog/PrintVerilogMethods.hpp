/// @file PrintVerilogMethods.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif2verilogParseLine.hpp"
#include <hif/hif.hpp>

/// @name Printing-related methods.
/// @{

/// @brief Generate header file(s).
/// @param sys The tree root.
/// @param outDir The output directory.
void printVerilog(hif::System *sys, hif2verilogParseLine &cLine);

/// @}

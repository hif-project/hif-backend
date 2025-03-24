/// @file PrintMethods.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

/// @name Printing-related methods.
/// @{

/// @brief Generate header file(s).
/// @param sys The tree root.
/// @param outDir The output directory.
void printVHDL(hif::System *sys, const std::string &outDir);

/// @}

/// @name File-related methods.
/// @{

void splitFileName(const std::string &f, std::string &base, std::string &ext);

/// @brief: Open and return stream associated to a specific file.
auto openFileStream(const std::string &name, std::ofstream *stream) -> int;

/// @brief: Close the output stream.
void closeFileStream(std::ofstream *stream);

/// @}

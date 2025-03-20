/// @file PrintMethods.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2verilog/PrintMethods.hpp"
#include "hif2verilog/VerilogPrinter.hpp"

using namespace hif;

void printVHDL(hif::System *sys, const std::string &outDir)
{
    messageAssert(sys != nullptr, "Given tree root is not valid", nullptr, nullptr);

    VerilogPrinter p(outDir);
    sys->acceptVisitor(p);
}

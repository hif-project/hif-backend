/// @file hif2verilogParseLine.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

class hif2verilogParseLine : public hif::application_utils::CommandLineParser
{
public:
    hif2verilogParseLine(int argc, char **argv);
    ~hif2verilogParseLine() override;

private:
    void _validateArguments();

    hif2verilogParseLine(const hif2verilogParseLine &);
    auto operator=(const hif2verilogParseLine &) -> hif2verilogParseLine &;
};

/// @file hif2vhdlParseLine.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

class hif2vhdlParseLine : public hif::application_utils::CommandLineParser
{
public:
    hif2vhdlParseLine(int argc, char *argv[]);
    ~hif2vhdlParseLine();

    int &getStep();

protected:
    void _validateArguments();

    int _step;

private:
    hif2vhdlParseLine(const hif2vhdlParseLine &);
    hif2vhdlParseLine &operator=(const hif2vhdlParseLine &);
};

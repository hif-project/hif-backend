/// @file hif2verilogParseLine.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <cstdlib>
#include <cstring>

#include "hif2verilog/hif2verilogParseLine.hpp"

hif2verilogParseLine::hif2verilogParseLine(int argc, char **argv)
    : CommandLineParser()
{
    addToolInfos(
        // Tool name.
        "hif2verilog",
        // Copyright.
        "Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group, Univeristy of Verona."
        "This file is distributed under the BSD 2-Clause License.",
        // description
        "Generates Verilog from a HIF description.",
        // synopsys
        "hif2verilog [OPTIONS] <HIF FILE>",
        // notes
        "- The default output directory name is hif2verilog_out.\n"
        "\n"
        "Email: info@hifsuite.com    Site: www.hifsuite.com");

    addHelp();
    addVersion();
    addVerbose();
    addOutputDirectory();
    addAutostep();
    addOptimization();

    addOption('s', "step", true, true, "Start translation from specified step.");

    parse(argc, argv);

    _validateArguments();
}

void hif2verilogParseLine::_validateArguments()
{
    if (!_options['h'].value.empty())
        printHelp();
    if (!_options['v'].value.empty())
        printVersion();

    if (_files.empty()) {
        messageError(
            "HIF input file missing.\n"
            "Try 'hif2verilog --help' for more information",
            nullptr, nullptr);
    }

    // Validate input file list
    if (_files.size() != 1) {
        messageError(
            "Required exactly one input file.\nTry hif2verilog --help "
            "for more infos.",
            nullptr, nullptr);
    }

    // Autostep management
    if (isAutostep()) {
        // ntd
    }

    // Establish output file name
    std::string out = _options['D'].value;
    if (out == "") {
        out = "hif2verilog_out";
    }
    _options['D'].value = out;
}

hif2verilogParseLine::~hif2verilogParseLine() {}

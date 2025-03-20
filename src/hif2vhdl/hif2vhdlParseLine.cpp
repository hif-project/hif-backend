/// @file hif2vhdlParseLine.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2vhdl/hif2vhdlParseLine.hpp"

hif2vhdlParseLine::hif2vhdlParseLine(int argc, char *argv[])
    : CommandLineParser()
    , _step(-1)
{
    addToolInfos(
        // Tool name.
        "hif2vhdl",
        // Copyright.
        "Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group, Univeristy of Verona."
        "This file is distributed under the BSD 2-Clause License.",
        // description
        "Generates VHDL from a HIF description.",
        // synopsys
        "hif2vhdl [OPTIONS] <HIF FILE>",
        // notes
        "- The default output directory name is hif2vhdl_out.\n"
        "\n"
        "Email: info@hifsuite.com    Site: www.hifsuite.com");

    addHelp();
    addVersion();
    addVerbose();
    addOutputDirectory();
    addPrintOnly();
    addAutostep();

    parse(argc, argv);

    _validateArguments();
}

hif2vhdlParseLine::~hif2vhdlParseLine()
{
    // ntd
}

void hif2vhdlParseLine::_validateArguments()
{
    if (!_options['h'].value.empty())
        printHelp();
    if (!_options['v'].value.empty())
        printVersion();

    if (_files.empty()) {
        messageError(
            "HIF input file missing.\n"
            "Try 'hif2vhdl --help' for more information",
            nullptr, nullptr);
    }

    // Validate input file list
    if (_files.size() != 1) {
        messageError(
            "Required exactly one input file.\nTry hif2vhdl --help "
            "for more infos.",
            nullptr, nullptr);
    }

    // Autostep management
    if (isAutostep()) {
        std::string filename(getOption('a'));
        const size_t lastSlashIdx = filename.find_last_of("\\/");
        if (std::string::npos != lastSlashIdx) {
            filename.erase(0, lastSlashIdx + 1);
        }

        std::string::size_type size = hif::application_utils::getApplicationName().size();
        filename                    = filename.substr(size + 1);

        std::stringstream s;
        s << filename;
        s >> _step;
        ++_step;
    }

    // Establish output dir name
    std::string out = _options['D'].value;
    if (out == "") {
        out = "hif2vhdl_out";
    }
    _options['D'].value = out;
}

int &hif2vhdlParseLine::getStep() { return _step; }

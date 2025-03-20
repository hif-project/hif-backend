/// @file hif2scParseLine.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2sc/hif2scParseLine.hpp"

hif2scParseLine::hif2scParseLine(int argc, char *argv[])
    : CommandLineParser()
    , _useResolved(false)
    , _useHDTLib(false)
    , _useCpp98(false)
    , _keepBit(false)
    , _noRealEquals(false)
    , _maxLines(0)
    , _maxWhen(5)
    , _sourcesExtension("cpp")
    , _headersExtension("hpp")
{
    addToolInfos(
        // Tool name.
        "hif2sc",
        // Copyright.
        "Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group, Univeristy of Verona."
        "This file is distributed under the BSD 2-Clause License.",
        // description
        "Generates SystemC (RTL/TLM/AMS)/C++/C from a HIF description.",
        // synopsys
        "hif2sc [OPTIONS] <HIF FILE>",
        // notes
        "- The default output directory name is hif2sc_out.\n"
        "\n"
        "Email: info@hifsuite.com    Site: www.hifsuite.com");

    addHelp();
    addVersion();
    addVerbose();
    addOutputDirectory();
    addAutostep();
    addOptimization();

#ifdef NDEBUG
    const bool isInternalRelease = false;
#else
    const bool isInternalRelease = true;
#endif

    addOption('b', "bit", false, true, "Keep deprecated sc_bit type. Default is mapping to bool.");

    addOption(
        'R', "resolved", false, isInternalRelease,
        "(Experimental) Preserve resolved signals and ports. Default is "
        "mapping to unresolved.");

    addOption('H', "hdtlib", false, true, "Use HDTLib data types instead of SystemC data types.");

    addOption('S', "cpp11", false, isInternalRelease, "Generate code compliant with C++11 standard; default is C++98.");

    addOption('s', "step", true, true, "Start translation from specified step.");

    addOption(
        'c', "compare-reals", false, true,
        "Preserve equality comparison between real values. Default is "
        "introducing a safe support method.");

    std::string maxLines;
    std::string maxWhen;
    std::stringstream ss1;
    ss1 << _maxLines;
    ss1 >> maxLines;
    std::stringstream ss2;
    ss2 << _maxWhen;
    ss2 >> maxWhen;

    addOption(
        'l', "lines", true, true,
        "Split generated files longer than specified value. 0 means "
        "never to split. "
        "Default is " +
            maxLines + ".");

    addOption(
        'w', "when", true, true,
        "Split ternary operators nested more than specified value. 0 "
        "means never to split. "
        "Default is " +
            maxWhen + ".");

    addOption(
        'e', "extensions", true, true,
        "In case of C++ files, append the specified file extensions to "
        "generated source and header file names. The file extensions must "
        "be specified in the following format: "
        "<sourceExtension>_<headerExtension>. "
        "E.g. cpp_hpp. Default is cc_hh.");

    parse(argc, argv);

    _validateArguments();
}

hif2scParseLine::~hif2scParseLine()
{
    // ntd
}

void hif2scParseLine::_validateArguments()
{
    if (!_options['h'].value.empty())
        printHelp();
    if (!_options['v'].value.empty())
        printVersion();

    if (_files.empty()) {
        messageError(
            "HIF input file missing.\n"
            "Try 'hif2sc --help' for more information",
            nullptr, nullptr);
    }

    // Validate input file list
    if (_files.size() != 1) {
        messageError(
            "Required exactly one input file.\nTry hif2sc --help for "
            "more infos.",
            nullptr, nullptr);
    }

    // Autostep management
    if (isAutostep()) {
        // ntd
    }

    if (!isActiveOption('R') && !_options['R'].value.empty()) {
        messageWarning(
            std::string("Option '-") + 'R' +
                "' is not available in this tool version."
                " It will be ignored.",
            nullptr, nullptr);
    }

    // Establish output file name
    std::string out = _options['D'].value;
    if (out == "") {
        out = "hif2sc_out";
    }
    _options['D'].value = out;

    if (!_options['l'].value.empty()) {
        std::stringstream ss1;
        ss1 << _options['l'].value;
        ss1 >> _maxLines;
    }

    if (!_options['w'].value.empty()) {
        std::stringstream ss1;
        ss1 << _options['w'].value;
        ss1 >> _maxWhen;
    }

    if (!_options['e'].value.empty()) {
        std::string ext   = _options['e'].value;
        std::size_t index = ext.find('_');
        if (index == std::string::npos) {
            messageError(
                "Invalid extensions format.\n"
                "Try 'hif2sc --help' for more information",
                nullptr, nullptr);
        }

        _sourcesExtension = ext.substr(0, index);
        _headersExtension = ext.substr(index + 1);
    }
}

bool hif2scParseLine::useResolved() const { return isOptionFlagSet('R'); }

bool hif2scParseLine::useHDTLib() const { return isOptionFlagSet('H'); }

bool hif2scParseLine::useCpp98() const { return !isOptionFlagSet('S'); }

bool hif2scParseLine::keepBit() const { return isOptionFlagSet('b'); }

bool hif2scParseLine::noRealEquals() const { return isOptionFlagSet('c'); }

uint64_t hif2scParseLine::getMaxLines() const { return _maxLines; }

uint64_t hif2scParseLine::getMaxWhen() const { return _maxWhen; }

std::string hif2scParseLine::getSourcesExtension() const { return _sourcesExtension; }

std::string hif2scParseLine::getHeadersExtension() const { return _headersExtension; }

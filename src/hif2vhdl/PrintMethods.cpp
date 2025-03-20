/// @file PrintMethods.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2vhdl/PrintMethods.hpp"
#include "hif2vhdl/VHDLPrinter.hpp"

using namespace hif;
using std::ofstream;

void printVHDL(hif::System *sys, const std::string &outDir)
{
    messageAssert(sys != nullptr, "Given tree root is not valid", nullptr, nullptr);

    VHDLPrinter p(outDir);
    sys->acceptVisitor(p);
}

void splitFileName(const std::string &f, std::string &base, std::string &ext)
{
    std::string::size_type p = f.find_last_of('.');
    if (p == std::string::npos) {
        base = f;
        ext  = "";
        return;
    }

    base = f.substr(0, p);
    ext  = f.substr(p + 1);
}

int openFileStream(const std::string &name, ofstream *outstream)
{
    outstream->open(name.c_str());
    if (!outstream->is_open() || !outstream->good())
        return 0;
    return 1;
}

void closeFileStream(ofstream *stream) { stream->close(); }

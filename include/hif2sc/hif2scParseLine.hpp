/// @file hif2scParseLine.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <inttypes.h>

#include <hif/hif.hpp>

class hif2scParseLine : public hif::application_utils::CommandLineParser
{
public:
    hif2scParseLine(int argc, char *argv[]);
    virtual ~hif2scParseLine();

    bool useResolved() const;
    bool useHDTLib() const;
    bool useCpp98() const;
    bool keepBit() const;
    bool noRealEquals() const;
    uint64_t getMaxLines() const;
    uint64_t getMaxWhen() const;
    std::string getSourcesExtension() const;
    std::string getHeadersExtension() const;

protected:
    /// @brief Validates and configures the arguments.
    void _validateArguments();

    bool _useResolved;
    bool _useHDTLib;
    bool _useCpp98;
    bool _keepBit;
    bool _noRealEquals;
    uint64_t _maxLines;
    uint64_t _maxWhen;
    std::string _sourcesExtension;
    std::string _headersExtension;

private:
    hif2scParseLine(const hif2scParseLine &);
    hif2scParseLine &operator=(const hif2scParseLine &);
};

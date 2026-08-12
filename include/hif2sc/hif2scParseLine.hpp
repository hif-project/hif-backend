/// @file hif2scParseLine.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <cinttypes>

#include <hif/hif.hpp>

class hif2scParseLine : public hif::application_utils::CommandLineParser
{
public:
    hif2scParseLine(int argc, char *argv[]);
    ~hif2scParseLine() override;

    hif2scParseLine(const hif2scParseLine &)                     = delete;
    auto operator=(const hif2scParseLine &) -> hif2scParseLine & = delete;

    auto useResolved() const -> bool;
    auto useHDTLib() const -> bool;
    auto useCpp98() const -> bool;
    auto keepBit() const -> bool;
    auto noRealEquals() const -> bool;
    auto getMaxLines() const -> uint64_t;
    auto getMaxWhen() const -> uint64_t;
    auto getSourcesExtension() const -> std::string;
    auto getHeadersExtension() const -> std::string;

private:
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
};

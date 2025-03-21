/// @file hif_vhdl_standard.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <iostream>

#include "hif2scSupport/hif_vhdl_standard.hpp"

namespace hif_vhdl_standard
{

void hif_vhdl_assert(const bool condition, const std::string &report, const hif_vhdl_severity_level level)
{
    if (condition) {
        return;
    }

    switch (level) {
    case hif_vhdl_note:
        std::cout << "[NOTE] " << report << '\n';
        break;
    case hif_vhdl_warning:
        std::clog << "[WARNING] " << report << '\n';
        break;
    case hif_vhdl_error:
        std::clog << "[ERROR] " << report << '\n';
        break;
    case hif_vhdl_failure:
        std::cerr << "[FAILURE] " << report << '\n';
        exit(1);
    default:
        return;
    }
}

auto hif_vhdl_castRealToInt(double param, int size, bool sign) -> long long int
{
#if (defined _MSC_VER)
    double d = floor(param + 0.5);
#else
    double d = round(param);
#endif

    double min = 0.0;
    double max = 0.0;
    if (!sign) {
        min = 0.0;
        max = pow(2.0, size) - 1;
    } else {
        min = -pow(2.0, size - 1);
        max = pow(2.0, size - 1) - 1;
    }

    if (d < min) {
        d = min;
    } else if (d > max) {
        d = max;
    }

    return static_cast<long long int>(d);
}

} // namespace hif_vhdl_standard

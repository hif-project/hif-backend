/// @file hif_vhdl_ieee_numeric_std.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2scSupport/hif_vhdl_ieee_numeric_std.hpp"

namespace hif_vhdl_ieee_numeric_std
{

// /////////////////////////////////////////////////////////////////////////////
// Conversion operators
// /////////////////////////////////////////////////////////////////////////////

auto hif_vhdl_to_integer_signed(const sc_dt::sc_lv_base &param1) -> int32_t
{
    if (!param1.is_01()) {
        return 0;
    }
    return param1.to_int();
}

HIF2SCSUPPORT_EXPORT
auto hif_vhdl_to_integer_unsigned(const sc_dt::sc_lv_base &param1) -> uint32_t
{
    if (!param1.is_01()) {
        return 0;
    }
    return param1.to_uint();
}

// /////////////////////////////////////////////////////////////////////////////
// Support methods
// /////////////////////////////////////////////////////////////////////////////

auto hif_vhdl_std_match(const sc_dt::sc_logic &param1, const sc_dt::sc_logic &param2) -> bool
{
    return (param1 == param2) && (param1 != 'X') && (param1 != 'Z');
}

auto hif_vhdl_std_match(const sc_dt::sc_lv_base &param1, const sc_dt::sc_lv_base &param2) -> bool
{
    if (param1.length() != param2.length()) {
        return false;
    }
    if (!param1.is_01() || !param2.is_01()) {
        return false;
    }
    return (param1.to_string() == param2.to_string());
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
bool hif_vhdl_std_match(const hdtlib::hl_logic_t &param1, const hdtlib::hl_logic_t &param2)
{
    return (param1 == param2) && (param1 != 'X') && (param1 != 'Z');
}
#endif

} // namespace hif_vhdl_ieee_numeric_std

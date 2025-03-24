/// @file hif_vhdl_ieee_std_logic_1164.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2scSupport/hif_vhdl_ieee_std_logic_1164.hpp"

#if (defined _MSC_VER)
#pragma warning(disable : 4127)
#endif

namespace hif_vhdl_ieee_std_logic_1164
{

namespace /* anon */
{

auto logic_index(const char v) -> int
{
    switch (toupper(v)) {
    case 'X':
        return 1;
    case '0':
        return 2;
    case '1':
        return 3;
    case 'Z':
        return 4;
    default:
        assert(false);
        return -1;
    }
}

char resolution_table[9][9] = {
    {'U', 'U', 'U', 'U', 'U', 'U', 'U', 'U', 'U'}, {'U', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
    {'U', 'X', '0', 'X', '0', '0', '0', '0', 'X'}, {'U', 'X', 'X', '1', '1', '1', '1', '1', 'X'},
    {'U', 'X', '0', '1', 'Z', 'W', 'L', 'H', 'X'}, {'U', 'X', '0', '1', 'W', 'W', 'W', 'W', 'X'},
    {'U', 'X', '0', '1', 'L', 'W', 'L', 'W', 'X'}, {'U', 'X', '0', '1', 'H', 'W', 'W', 'H', 'X'},
    {'U', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'}};

} // namespace

auto hif_vhdl_resolved(const sc_dt::sc_lv_base &s) -> sc_dt::sc_logic
{
    if (s.length() == 1) {
        return static_cast<sc_dt::sc_logic>(s[0]);
    }

    sc_dt::sc_logic result('z');
    for (int i = 0; i < s.length(); ++i) {
        const int a = logic_index(result.to_char());
        const int b = logic_index(s[i].to_char());
        result      = resolution_table[a][b];
    }
    return result;
}

auto hif_vhdl_to_bit(const sc_dt::sc_logic &s, bool xmap) -> bool
{
    if (s == sc_dt::SC_LOGIC_1) {
        return true;
    }
    if (s == sc_dt::SC_LOGIC_0) {
        return false;
}
    return xmap;
}

auto hif_vhdl_to_x01(const sc_dt::sc_logic &s) -> sc_dt::sc_logic
{
    if (toupper(s.to_char()) != 'Z') {
        return s;
    }
    return sc_dt::SC_LOGIC_X;
}

auto hif_vhdl_to_ux01(const sc_dt::sc_logic &s) -> sc_dt::sc_logic { return hif_vhdl_to_x01(s); }

#ifdef HIF2SCSUPPORT_USE_HDTLIB
bool hif_vhdl_to_bit(const hdtlib::hl_logic_t s, bool xmap)
{
    sc_dt::sc_logic tmp(s.to_char());
    return hif_vhdl_to_bit(tmp, xmap);
}

hdtlib::hl_logic_t hif_vhdl_to_x01(const hdtlib::hl_logic_t &s)
{
    sc_dt::sc_logic tmp(s.to_char());
    sc_dt::sc_logic tmpRes = hif_vhdl_to_x01(tmp);
    hdtlib::hl_logic_t res = tmpRes.to_char();
    return res;
}

hdtlib::hl_logic_t hif_vhdl_to_ux01(const hdtlib::hl_logic_t &s)
{
    sc_dt::sc_logic tmp(s.to_char());
    sc_dt::sc_logic tmpRes = hif_vhdl_to_ux01(tmp);
    hdtlib::hl_logic_t res = tmpRes.to_char();
    return res;
}
#endif

auto hif_vhdl_is_x(const sc_dt::sc_logic &s) -> bool { return !s.is_01(); }

auto hif_vhdl_is_x(const sc_dt::sc_lv_base &s) -> bool { return !s.is_01(); }

#ifdef HIF2SCSUPPORT_USE_HDTLIB
bool hif_vhdl_is_x(const hdtlib::hl_logic_t s) { return !s.is_01(); }
#endif

} // namespace hif_vhdl_ieee_std_logic_1164

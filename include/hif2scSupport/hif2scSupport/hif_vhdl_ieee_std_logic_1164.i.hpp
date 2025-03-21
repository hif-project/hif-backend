/// @file hif_vhdl_ieee_std_logic_1164.i.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "../hif_vhdl_ieee_std_logic_1164.hpp"

namespace hif_vhdl_ieee_std_logic_1164
{

template <int size> auto hif_vhdl_to_bitvector(const sc_dt::sc_lv<size> &s, const bool xmap) -> sc_dt::sc_bv<size>
{
    std::string sVal = s.to_string();
    for (std::string::size_type i = 0; i < sVal.length(); ++i) {
        if (sVal[i] == '1' || sVal[i] == '0') {
            continue;
        }
        sVal[i] = xmap ? '1' : '0';
    }
    return sc_dt::sc_bv<size>(sVal.c_str());
}

template <int size> auto hif_vhdl_to_x01(const sc_dt::sc_lv<size> &s) -> sc_dt::sc_lv<size>
{
    std::string sVal = s.to_string();
    for (std::string::size_type i = 0; i < sVal.length(); ++i) {
        if (sVal[i] == '1' || sVal[i] == '0') {
            continue;
        }
        sVal[i] = 'X';
    }
    return sc_dt::sc_lv<size>(sVal.c_str());
}

template <int size> auto hif_vhdl_to_ux01(const sc_dt::sc_lv<size> &s) -> sc_dt::sc_lv<size>
{
    return hif_vhdl_to_x01(s);
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int size> hdtlib::hl_logic_t hif_vhdl_resolved(const hdtlib::hl_lv_t<size> &s)
{
    sc_dt::sc_lv<size> tmp = s.to_string().c_str();
    sc_dt::sc_logic tmpRes = hif_vhdl_resolved(tmp);
    hdtlib::hl_logic_t res = tmpRes.to_char();
    return res;
}

template <int size> hdtlib::hl_bv_t<size> hif_vhdl_to_bitvector(const hdtlib::hl_lv_t<size> &s, const bool /*xmap*/)
{
    sc_dt::sc_lv<size> tmp    = s.to_string().c_str();
    sc_dt::sc_bv<size> tmpRes = hif_vhdl_to_bitvector(tmp);
    hdtlib::hl_bv_t<size> res = tmpRes.to_string().c_str();
    return res;
}

template <int size> hdtlib::hl_lv_t<size> hif_vhdl_to_x01(const hdtlib::hl_lv_t<size> &s)
{
    sc_dt::sc_lv<size> tmp    = s.to_string().c_str();
    sc_dt::sc_lv<size> tmpRes = hif_vhdl_to_x01(tmp);
    hdtlib::hl_lv_t<size> res = tmpRes.to_string().c_str();
    return res;
}

template <int size> hdtlib::hl_lv_t<size> hif_vhdl_to_ux01(const hdtlib::hl_lv_t<size> &s)
{
    sc_dt::sc_lv<size> tmp    = s.to_string().c_str();
    sc_dt::sc_lv<size> tmpRes = hif_vhdl_to_ux01(tmp);
    hdtlib::hl_lv_t<size> res = tmpRes.to_string().c_str();
    return res;
}

template <int size> bool hif_vhdl_is_x(const hdtlib::hl_lv_t<size> s) { return !s.is_01(); }

#endif

} // namespace hif_vhdl_ieee_std_logic_1164

/// @file hif_vhdl_ieee_std_logic_signed.i.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "../hif_vhdl_ieee_std_logic_arith.hpp"
#include "../hif_vhdl_ieee_std_logic_signed.hpp"

namespace hif_vhdl_ieee_std_logic_signed
{

// /////////////////////////////////////////////////////////////////////////////
// Shift operators
// /////////////////////////////////////////////////////////////////////////////

template <int size1, int size2> sc_dt::sc_lv<size1> hif_vhdl_shl(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl_shl(arg, count);
}

template <int size1, int size2> sc_dt::sc_lv<size1> hif_vhdl_shr(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl_shr_signed(arg, count);
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shl(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl_shl(arg, count);
}

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shr(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl_shr_signed(arg, count);
}

#endif // HIF2SCSUPPORT_USE_HDTLIB

// /////////////////////////////////////////////////////////////////////////////
// Conversion operators
// /////////////////////////////////////////////////////////////////////////////

template <int size> long long int hif_vhdl_conv_integer(sc_dt::sc_lv<size> arg)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl_conv_integer_signed(arg);
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size> long long int hif_vhdl_conv_integer(hdtlib::hl_lv_t<size> arg)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl_conv_integer_signed(arg);
}

#endif // HIF2SCSUPPORT_USE_HDTLIB

// /////////////////////////////////////////////////////////////////////////////
// Relational operators
// /////////////////////////////////////////////////////////////////////////////

template <int size> bool hif_vhdl__op_eq(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_eq_signed(v1, v2);
}

template <int size> bool hif_vhdl__op_neq(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_neq_signed(v1, v2);
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size> bool hif_vhdl__op_eq(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_eq_signed(v1, v2);
}

template <int size> bool hif_vhdl__op_neq(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_neq_signed(v1, v2);
}

#endif // HIF2SCSUPPORT_USE_HDTLIB

// /////////////////////////////////////////////////////////////////////////////
// Arithmetic operators
// /////////////////////////////////////////////////////////////////////////////

template <int size> sc_dt::sc_lv<size> hif_vhdl__op_plus(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_plus_signed(v1, v2);
}

template <int size> sc_dt::sc_lv<size> hif_vhdl__op_minus(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_minus_signed(v1, v2);
}

template <int size1, int size2>
sc_dt::sc_lv<size1 + size2> hif_vhdl__op_mult(const sc_dt::sc_lv<size1> &v1, const sc_dt::sc_lv<size2> &v2)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_mult_signed(v1, v2);
}

template <int size> sc_dt::sc_lv<size> hif_vhdl__op_abs(sc_dt::sc_lv<size> arg)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_abs(arg);
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size>
hdtlib::hl_lv_t<size> hif_vhdl__op_plus(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_plus_signed(v1, v2);
}

template <int size>
hdtlib::hl_lv_t<size> hif_vhdl__op_minus(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_minus_signed(v1, v2);
}

template <int size1, int size2>
hdtlib::hl_lv_t<size1 + size2> hif_vhdl__op_mult(const hdtlib::hl_lv_t<size1> &v1, const hdtlib::hl_lv_t<size2> &v2)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_mult_signed(v1, v2);
}

template <int size> hdtlib::hl_lv_t<size> hif_vhdl__op_abs(hdtlib::hl_lv_t<size> arg)
{
    return hif_vhdl_ieee_std_logic_arith::hif_vhdl__op_abs(arg);
}

#endif // HIF2SCSUPPORT_USE_HDTLIB

} // namespace hif_vhdl_ieee_std_logic_signed

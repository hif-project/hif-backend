/// @file hif_vhdl_ieee_numeric_std.i.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "../hif_vhdl_ieee_numeric_std.hpp"

namespace hif_vhdl_ieee_numeric_std
{

// /////////////////////////////////////////////////////////////////////////////
// Conversion operators
// /////////////////////////////////////////////////////////////////////////////

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int W>
int32_t hif_vhdl_to_integer_signed(const hdtlib::hl_lv_t<W> &param1)
{
    if (!param1.is_01())
        return 0;
    return param1.to_int();
}

template <int W>
uint32_t hif_vhdl_to_integer_unsigned(const hdtlib::hl_lv_t<W> &param1)
{
    if (!param1.is_01())
        return 0;
    return param1.to_uint();
}

#endif

// /////////////////////////////////////////////////////////////////////////////
// Support methods
// /////////////////////////////////////////////////////////////////////////////

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int W>
bool hif_vhdl_std_match(const hdtlib::hl_lv_t<W> &param1, const hdtlib::hl_lv_t<W> &param2)
{
    if (!param1.is_01() || !param2.is_01())
        return false;
    return (param1.to_string() == param2.to_string());
}

#endif

// /////////////////////////////////////////////////////////////////////////////
// Arithmetic operators
// /////////////////////////////////////////////////////////////////////////////

template <int size>
sc_dt::sc_lv<size> hif_vhdl__op_abs(sc_dt::sc_lv<size> arg)
{
    sc_dt::sc_lv<size> x;
    if (!arg.is_01())
        return x;

    sc_dt::sc_bigint<size> tmp(static_cast<sc_dt::sc_bigint<size>>(arg));
    if (tmp < 0)
        tmp = -tmp;

    return static_cast<sc_dt::sc_lv<size>>(tmp);
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size>
hdtlib::hl_lv_t<size> hif_vhdl__op_abs(hdtlib::hl_lv_t<size> arg)
{
    hdtlib::hl_lv_t<size> x;
    if (!arg.is_01())
        return x;

    hdtlib::hl_int_t<size> tmp(static_cast<hdtlib::hl_int_t<size>>(arg));
    if (tmp < 0)
        tmp = -tmp;

    return static_cast<hdtlib::hl_lv_t<size>>(tmp);
}

#endif // HIF2SCSUPPORT_USE_HDTLIB

// /////////////////////////////////////////////////////////////////////////////
// Relational operators
// /////////////////////////////////////////////////////////////////////////////
template <int size>
bool hif_vhdl__op_eq_signed(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return false;
    return v1 == v2;
}

template <int size>
bool hif_vhdl__op_neq_signed(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2)
{
    return !hif_vhdl__op_eq_signed(v1, v2);
}

template <typename T, size_t size>
bool hif_vhdl__op_eq_signed(T (&v1)[size], T (&v2)[size])
{
    bool res = hif_vhdl__op_eq_signed(v1[0], v2[0]);
    for (size_t i = 1; i < size; ++i) {
        res &= hif_vhdl__op_eq_signed(v1[i], v2[i]);
    }

    return res;
}

template <typename T, size_t size>
bool hif_vhdl__op_neq_signed(T (&v1)[size], T (&v2)[size])
{
    return !hif_vhdl__op_eq_signed(v1, v2);
}

template <int size>
bool hif_vhdl__op_eq_unsigned(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return false;
    return v1 == v2;
}

template <int size>
bool hif_vhdl__op_neq_unsigned(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2)
{
    return !hif_vhdl__op_eq_unsigned(v1, v2);
}

template <typename T, size_t size>
bool hif_vhdl__op_eq_unsigned(T (&v1)[size], T (&v2)[size])
{
    bool res = hif_vhdl__op_eq_unsigned(v1[0], v2[0]);
    for (size_t i = 1; i < size; ++i) {
        res &= hif_vhdl__op_eq_unsigned(v1[i], v2[i]);
    }

    return res;
}

template <typename T, size_t size>
bool hif_vhdl__op_neq_unsigned(T (&v1)[size], T (&v2)[size])
{
    return !hif_vhdl__op_eq_unsigned(v1, v2);
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size>
bool hif_vhdl__op_eq_signed(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return false;
    return v1 == v2;
}

template <int size>
bool hif_vhdl__op_eq_signed(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    if (!v1.is_01())
        return false;
    return v1 == v2;
}

template <int size>
bool hif_vhdl__op_eq_signed(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    if (!v2.is_01())
        return false;
    return v1 == v2;
}

template <int size>
bool hif_vhdl__op_eq_signed(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return v1 == v2;
}

template <int size>
bool hif_vhdl__op_neq_signed(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return !hif_vhdl__op_eq_signed(v1, v2);
}

template <int size>
bool hif_vhdl__op_neq_signed(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return !hif_vhdl__op_eq_signed(v1, v2);
}

template <int size>
bool hif_vhdl__op_neq_signed(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return !hif_vhdl__op_eq_signed(v1, v2);
}

template <int size>
bool hif_vhdl__op_neq_signed(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return !hif_vhdl__op_eq_signed(v1, v2);
}

template <int size>
bool hif_vhdl__op_eq_unsigned(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return false;
    return v1 == v2;
}

template <int size>
bool hif_vhdl__op_eq_unsigned(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    if (!v1.is_01())
        return false;
    return v1 == v2;
}

template <int size>
bool hif_vhdl__op_eq_unsigned(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    if (!v2.is_01())
        return false;
    return v1 == v2;
}

template <int size>
bool hif_vhdl__op_eq_unsigned(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return v1 == v2;
}

template <int size>
bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return !hif_vhdl__op_eq_unsigned(v1, v2);
}

template <int size>
bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return !hif_vhdl__op_eq_unsigned(v1, v2);
}

template <int size>
bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return !hif_vhdl__op_eq_unsigned(v1, v2);
}

template <int size>
bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return !hif_vhdl__op_eq_unsigned(v1, v2);
}

#endif // HIF2SCSUPPORT_USE_HDTLIB

} // namespace hif_vhdl_ieee_numeric_std

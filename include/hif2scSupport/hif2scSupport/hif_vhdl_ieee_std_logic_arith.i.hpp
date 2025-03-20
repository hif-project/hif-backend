/// @file hif_vhdl_ieee_std_logic_arith.i.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "../hif_vhdl_ieee_std_logic_arith.hpp"
#include <algorithm>

namespace hif_vhdl_ieee_std_logic_arith
{

// /////////////////////////////////////////////////////////////////////////////
// Relational operators
// /////////////////////////////////////////////////////////////////////////////

template <int size>
bool hif_vhdl__op_eq_signed(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2)
{
    return (hif_vhdl_conv_signed_signed<size>(v1) == hif_vhdl_conv_signed_signed<size>(v2));
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
    return (hif_vhdl_conv_unsigned_unsigned<size>(v1) == hif_vhdl_conv_unsigned_unsigned<size>(v2));
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
    return (hif_vhdl_conv_signed_unsigned<size>(v1) == hif_vhdl_conv_signed_unsigned<size>(v2));
}

template <int size>
bool hif_vhdl__op_eq_signed(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return (hif_vhdl_conv_signed_unsigned<size>(v1) == hif_vhdl_conv_signed_unsigned<size>(hdtlib::hl_lv_t<size>(v2)));
}

template <int size>
bool hif_vhdl__op_eq_signed(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return (hif_vhdl_conv_signed_unsigned<size>(hdtlib::hl_lv_t<size>(v1)) == hif_vhdl_conv_signed_unsigned<size>(v2));
}

template <int size>
bool hif_vhdl__op_eq_signed(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return (
        hif_vhdl_conv_signed_unsigned<size>(hdtlib::hl_lv_t<size>(v1)) ==
        hif_vhdl_conv_signed_unsigned<size>(hdtlib::hl_lv_t<size>(v2)));
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
    return (hif_vhdl_conv_unsigned_unsigned<size>(v1) == hif_vhdl_conv_unsigned_unsigned<size>(v2));
}

template <int size>
bool hif_vhdl__op_eq_unsigned(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return (
        hif_vhdl_conv_unsigned_unsigned<size>(v1) == hif_vhdl_conv_unsigned_unsigned<size>(hdtlib::hl_lv_t<size>(v2)));
}

template <int size>
bool hif_vhdl__op_eq_unsigned(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return (
        hif_vhdl_conv_unsigned_unsigned<size>(hdtlib::hl_lv_t<size>(v1)) == hif_vhdl_conv_unsigned_unsigned<size>(v2));
}

template <int size>
bool hif_vhdl__op_eq_unsigned(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return (
        hif_vhdl_conv_unsigned_unsigned<size>(hdtlib::hl_lv_t<size>(v1)) ==
        hif_vhdl_conv_unsigned_unsigned<size>(hdtlib::hl_lv_t<size>(v2)));
}

template <int size>
bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return !hif_vhdl__op_eq_unsigned(v1, v2);
}

template <int size>
bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2)
{
    return !hif_vhdl__op_eq_unsigned(v1, v2);
}

template <int size>
bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return !hif_vhdl__op_eq_unsigned(v1, v2);
}

template <int size>
bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2)
{
    return !hif_vhdl__op_eq_unsigned(v1, v2);
}

#endif // HIF2SCSUPPORT_USE_HDTLIB

// /////////////////////////////////////////////////////////////////////////////
// Arithmetic operators
// /////////////////////////////////////////////////////////////////////////////

template <int size, template <int> class T1, template <int> class T2>
typename RetType<size, T1>::Type hif_vhdl__op_plus_signed(const T1<size> &v1, const T2<size> &v2)
{
    typedef typename RetType<size, T1>::Type Type;
    typedef typename RetType<size, T1>::Int Int;

    Type ret;
    if (!v1.is_01() || !v2.is_01())
        return ret;
    ret = static_cast<Type>(static_cast<Int>(v1) + static_cast<Int>(v2));
    return ret;
}

template <int size, template <int> class T1, template <int> class T2>
typename RetType<size, T1>::Type hif_vhdl__op_minus_signed(const T1<size> &v1, const T2<size> &v2)
{
    typedef typename RetType<size, T1>::Type Type;
    typedef typename RetType<size, T1>::Int Int;

    Type ret;
    if (!v1.is_01() || !v2.is_01())
        return ret;
    ret = static_cast<Type>(static_cast<Int>(v1) - static_cast<Int>(v2));
    return ret;
}

template <int size1, int size2, template <int> class T1, template <int> class T2>
typename RetType<size1 + size2, T1>::Type hif_vhdl__op_mult_signed(const T1<size1> &v1, const T2<size2> &v2)
{
    typedef typename RetType<size1 + size2, T1>::Type Type;
    typedef typename RetType<size1 + size2, T1>::Int Int;

    typedef typename RetType<size1, T1>::Int Int1;
    typedef typename RetType<size2, T1>::Int Int2;

    Type ret;
    if (!v1.is_01() || !v2.is_01())
        return ret;
    ret = static_cast<Type>(static_cast<Int>(static_cast<Int1>(v1)) * static_cast<Int>(static_cast<Int2>(v2)));
    return ret;
}

template <int size, template <int> class T1, template <int> class T2>
typename RetType<size, T1>::Type hif_vhdl__op_plus_unsigned(const T1<size> &v1, const T2<size> &v2)
{
    typedef typename RetType<size, T1>::Type Type;
    typedef typename RetType<size, T1>::UInt UInt;

    Type ret;
    if (!v1.is_01() || !v2.is_01())
        return ret;
    ret = static_cast<Type>(static_cast<UInt>(v1) + static_cast<UInt>(v2));
    return ret;
}

template <int size, template <int> class T1, template <int> class T2>
typename RetType<size, T1>::Type hif_vhdl__op_minus_unsigned(const T1<size> &v1, const T2<size> &v2)
{
    typedef typename RetType<size, T1>::Type Type;
    typedef typename RetType<size, T1>::UInt UInt;

    Type ret;
    if (!v1.is_01() || !v2.is_01())
        return ret;
    ret = static_cast<Type>(static_cast<UInt>(v1) - static_cast<UInt>(v2));
    return ret;
}

template <int size1, int size2, template <int> class T1, template <int> class T2>
typename RetType<size1 + size2, T1>::Type hif_vhdl__op_mult_unsigned(const T1<size1> &v1, const T2<size2> &v2)
{
    typedef typename RetType<size1 + size2, T1>::Type Type;
    typedef typename RetType<size1 + size2, T1>::UInt UInt;

    Type ret;
    if (!v1.is_01() || !v2.is_01())
        return ret;
    ret = static_cast<Type>(static_cast<UInt>(v1) * static_cast<UInt>(v2));
    return ret;
}

template <int size, template <int> class T1>
typename RetType<size, T1>::Type hif_vhdl__op_abs(T1<size> arg)
{
    typedef typename RetType<size, T1>::Type Type;
    typedef typename RetType<size, T1>::Int Int;

    Type x;
    if (!arg.is_01())
        return x;

    Int tmp(static_cast<Int>(arg));
    if (tmp < 0)
        tmp = -tmp;

    return static_cast<Type>(tmp);
}

// /////////////////////////////////////////////////////////////////////////////
// Sign-extension operators
// /////////////////////////////////////////////////////////////////////////////

template <int size2, int size1>
sc_dt::sc_lv<size2> hif_vhdl_sxt(sc_dt::sc_lv<size1> arg)
{
    if (size2 <= 0)
        return sc_dt::sc_lv<size2>();
    if (!arg.is_01())
        return sc_dt::sc_lv<size2>('X');
    if (size2 <= arg.length())
        return arg.range(size2 - 1, 0);

    std::string xStr = arg.to_string();
    char c           = arg[arg.length() - 1].to_char();
    for (int i = arg.length(); i < size2; ++i) {
        xStr = c + xStr;
    }
    return sc_dt::sc_lv<size2>(xStr.c_str());
}

template <int size2, int size1>
sc_dt::sc_lv<size2> hif_vhdl_ext(sc_dt::sc_lv<size1> arg)
{
    if (size2 <= 0)
        return sc_dt::sc_lv<size2>();
    if (!arg.is_01())
        return sc_dt::sc_lv<size2>('X');
    if (size2 <= arg.length())
        return arg.range(size2 - 1, 0);

    std::string xStr = arg.to_string();
    for (int i = arg.length(); i < size2; ++i) {
        xStr = '0' + xStr;
    }
    return sc_dt::sc_lv<size2>(xStr.c_str());
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size2, int size1>
hdtlib::hl_lv_t<size2> hif_vhdl_sxt(hdtlib::hl_lv_t<size1> arg)
{
    if (size2 <= 0)
        return hdtlib::hl_lv_t<size2>();
    if (!arg.is_01())
        return hdtlib::hl_lv_t<size2>('X');
    if (size2 <= size1)
        return arg.template range<size2>(size2 - 1, 0);

    std::string xStr = arg.to_string();
    char c           = arg[size1 - 1].to_char();
    for (int i = size1; i < size2; ++i) {
        xStr = c + xStr;
    }
    return hdtlib::hl_lv_t<size2>(xStr.c_str());
}

template <int size2, int size1>
hdtlib::hl_lv_t<size2> hif_vhdl_sxt(hdtlib::hl_bv_t<size1> arg)
{
    return hif_vhdl_sxt(hdtlib::hl_lv_t<size1>(arg));
}

template <int size2, int size1>
hdtlib::hl_lv_t<size2> hif_vhdl_ext(hdtlib::hl_lv_t<size1> arg)
{
    if (size2 <= 0)
        return hdtlib::hl_lv_t<size2>();
    if (!arg.is_01())
        return hdtlib::hl_lv_t<size2>('X');
    if (size2 <= size1)
        return arg.template range<size1>(size2 - 1, 0);

    std::string xStr = arg.to_string();
    for (int i = size1; i < size2; ++i) {
        xStr = '0' + xStr;
    }
    return hdtlib::hl_lv_t<size2>(xStr.c_str());
}

template <int size2, int size1>
hdtlib::hl_lv_t<size2> hif_vhdl_ext(hdtlib::hl_bv_t<size1> arg)
{
    return hif_vhdl_ext(hdtlib::hl_lv_t<size1>(arg));
}

#endif // HIF2SCSUPPORT_USE_HDTLIB

// /////////////////////////////////////////////////////////////////////////////
// Shift operators
// /////////////////////////////////////////////////////////////////////////////

template <int size1, int size2>
sc_dt::sc_lv<size1> hif_vhdl_shl(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count)
{
    if (!count.is_01())
        return sc_dt::sc_lv<size1>(sc_dt::SC_LOGIC_X);

    sc_dt::sc_lv<size1> ret;
    ret = arg << count.to_int();
    return ret;
}

template <int size1, int size2>
sc_dt::sc_lv<size1> hif_vhdl_shr_signed(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count)
{
    if (!count.is_01())
        return sc_dt::sc_lv<size1>(sc_dt::SC_LOGIC_X);

    sc_dt::sc_logic signBit(arg[size1 - 1]);
    sc_dt::sc_lv<size1> sign(signBit);

    int i = size1 - count.to_int();
    if (i <= 0)
        return sign;

    sc_dt::sc_lv<size1> ret;
    ret = arg >> count.to_int();
    ret = (sign.range(size1 - 1, i), ret.range(i - 1, 0));
    return ret;
}

template <int size1, int size2>
sc_dt::sc_lv<size1> hif_vhdl_shr_unsigned(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count)
{
    if (!count.is_01())
        return sc_dt::sc_lv<size1>(sc_dt::SC_LOGIC_X);

    sc_dt::sc_logic signBit(sc_dt::SC_LOGIC_0);
    sc_dt::sc_lv<size1> sign(signBit);

    int i = static_cast<int>(size1 - count.to_uint());
    if (i <= 0)
        return sign;

    sc_dt::sc_lv<size1> ret;
    ret = arg >> count.to_int();
    ret = (sign.range(size1 - 1, i), ret.range(i - 1, 0));

    return ret;
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shl(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count)
{
    if (!count.is_01())
        return hdtlib::hl_lv_t<size1>('X');

    hdtlib::hl_lv_t<size1> ret;
    ret = arg << count.to_int();
    return ret;
}

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shl(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_bv_t<size2> count)
{
    return hif_vhdl_shl(arg, hdtlib::hl_lv_t<size2>(count));
}

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shr_signed(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count)
{
    // Exploiting systemc since .range() is template.

    if (!count.is_01())
        return hdtlib::hl_lv_t<size1>('X');

    sc_dt::sc_logic signBit(arg[size1 - 1].to_char());
    sc_dt::sc_lv<size1> sign(signBit);

    int i = size1 - count.to_int();
    if (i <= 0)
        return hdtlib::hl_lv_t<size1>(sign.to_string().c_str());

    sc_dt::sc_lv<size1> ret;
    ret = (arg >> count.to_int()).to_string().c_str();
    ret = (sign.range(size1 - 1, i), ret.range(i - 1, 0));

    return hdtlib::hl_lv_t<size1>(ret.to_string().c_str());
}

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shr_signed(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_bv_t<size2> count)
{
    return hif_vhdl_shr_signed(arg, hdtlib::hl_lv_t<size2>(count));
}

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shr_unsigned(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count)
{
    // Exploiting systemc since .range() is template.

    if (!count.is_01())
        return hdtlib::hl_lv_t<size1>('X');

    sc_dt::sc_logic signBit(sc_dt::SC_LOGIC_0);
    sc_dt::sc_lv<size1> sign(signBit);

    int i = static_cast<int>(size1 - count.to_uint());
    if (i <= 0)
        return hdtlib::hl_lv_t<size1>(sign.to_string().c_str());

    sc_dt::sc_lv<size1> ret;
    ret = (arg >> count.to_int()).to_string().c_str();
    ret = (sign.range(size1 - 1, i), ret.range(i - 1, 0));

    return hdtlib::hl_lv_t<size1>(ret.to_string().c_str());
}

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shr_unsigned(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_bv_t<size2> count)
{
    return hif_vhdl_shr_unsigned(arg, hdtlib::hl_lv_t<size2>(count));
}

#endif // HIF2SCSUPPORT_USE_HDTLIB

// /////////////////////////////////////////////////////////////////////////////
// Conversion operators
// /////////////////////////////////////////////////////////////////////////////

template <int size>
long long int hif_vhdl_conv_integer_signed(sc_dt::sc_lv<size> arg)
{
    // Modelsim implementation:
    if (!arg.is_01())
        return 0;
    return arg.to_int64();
}

template <int size>
long long int hif_vhdl_conv_integer_unsigned(sc_dt::sc_lv<size> arg)
{
    // Modelsim implementation:
    if (!arg.is_01())
        return 0;
    return arg.to_uint64();
}

template <int newsize, int size>
sc_dt::sc_lv<newsize> hif_vhdl_conv_std_logic_vector_signed(sc_dt::sc_lv<size> arg)
{
    if (!arg.is_01())
        return sc_dt::sc_lv<newsize>('X');
    return hif_vhdl_sxt<newsize>(arg);
}

template <int newsize, int size>
sc_dt::sc_lv<newsize> hif_vhdl_conv_std_logic_vector_unsigned(sc_dt::sc_lv<size> arg)
{
    if (!arg.is_01())
        return sc_dt::sc_lv<newsize>('X');
    return sc_dt::sc_lv<newsize>(arg); // zero-extension
}

template <int newsize>
sc_dt::sc_lv<newsize> hif_vhdl_conv_std_logic_vector(sc_dt::sc_logic arg)
{
    if (!arg.is_01())
        return sc_dt::sc_lv<newsize>('X');
    return sc_dt::sc_lv<newsize>(sc_dt::sc_lv<1>(arg)); // zero-extension
}

template <int newsize, int size>
sc_dt::sc_lv<newsize> hif_vhdl_conv_signed_signed(sc_dt::sc_lv<size> arg)
{
    if (!arg.is_01())
        return sc_dt::sc_lv<newsize>('X');
    return hif_vhdl_sxt<newsize>(arg);
}

template <int newsize, int size>
sc_dt::sc_lv<newsize> hif_vhdl_conv_signed_unsigned(sc_dt::sc_lv<size> arg)
{
    if (!arg.is_01())
        return sc_dt::sc_lv<newsize>('X');
    return sc_dt::sc_lv<newsize>(arg); // zero-extension
}

template <int newsize>
sc_dt::sc_lv<newsize> hif_vhdl_conv_signed(sc_dt::sc_logic arg)
{
    if (!arg.is_01())
        return sc_dt::sc_lv<newsize>('X');
    return sc_dt::sc_lv<newsize>(sc_dt::sc_lv<1>(arg)); // zero-extension
}

template <int newsize, int size>
sc_dt::sc_lv<newsize> hif_vhdl_conv_unsigned_signed(sc_dt::sc_lv<size> arg)
{
    if (!arg.is_01())
        return sc_dt::sc_lv<newsize>('X');
    return hif_vhdl_sxt<newsize>(arg);
}

template <int newsize, int size>
sc_dt::sc_lv<newsize> hif_vhdl_conv_unsigned_unsigned(sc_dt::sc_lv<size> arg)
{
    if (!arg.is_01())
        return sc_dt::sc_lv<newsize>('X');
    return sc_dt::sc_lv<newsize>(arg); // zero-extension
}

template <int newsize>
sc_dt::sc_lv<newsize> hif_vhdl_conv_unsigned(sc_dt::sc_logic arg)
{
    if (!arg.is_01())
        return sc_dt::sc_lv<newsize>('X');
    return sc_dt::sc_lv<newsize>(sc_dt::sc_lv<1>(arg)); // zero-extension
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size>
long long int hif_vhdl_conv_integer_signed(hdtlib::hl_lv_t<size> arg)
{
    // Modelsim implementation:
    if (!arg.is_01())
        return 0;
    return arg.to_int64();
}

template <int size>
long long int hif_vhdl_conv_integer_unsigned(hdtlib::hl_lv_t<size> arg)
{
    // Modelsim implementation:
    if (!arg.is_01())
        return 0;
    return arg.to_uint64();
}

template <int newsize, int size>
hdtlib::hl_lv_t<newsize> hif_vhdl_conv_std_logic_vector_signed(hdtlib::hl_lv_t<size> arg)
{
    if (!arg.is_01())
        return hdtlib::hl_lv_t<newsize>('X');
    return hif_vhdl_sxt<newsize>(arg);
}

template <int newsize, int size>
hdtlib::hl_lv_t<newsize> hif_vhdl_conv_std_logic_vector_unsigned(hdtlib::hl_lv_t<size> arg)
{
    if (!arg.is_01())
        return hdtlib::hl_lv_t<newsize>('X');
    return hdtlib::hl_lv_t<newsize>(arg); // zero-extension
}

template <int newsize>
hdtlib::hl_lv_t<newsize> hif_vhdl_conv_std_logic_vector(hdtlib::hl_logic_t arg)
{
    if (!arg.is_01())
        return hdtlib::hl_lv_t<newsize>('X');
    return hdtlib::hl_lv_t<newsize>(hdtlib::hl_lv_t<1>(arg)); // zero-extension
}

template <int newsize, int size>
hdtlib::hl_lv_t<newsize> hif_vhdl_conv_signed_signed(hdtlib::hl_lv_t<size> arg)
{
    if (!arg.is_01())
        return hdtlib::hl_lv_t<newsize>('X');
    return hif_vhdl_sxt<newsize>(arg);
}

template <int newsize, int size>
hdtlib::hl_lv_t<newsize> hif_vhdl_conv_signed_unsigned(hdtlib::hl_lv_t<size> arg)
{
    if (!arg.is_01())
        return hdtlib::hl_lv_t<newsize>('X');
    return hdtlib::hl_lv_t<newsize>(arg); // zero-extension
}

template <int newsize>
hdtlib::hl_lv_t<newsize> hif_vhdl_conv_signed(hdtlib::hl_logic_t arg)
{
    if (!arg.is_01())
        return hdtlib::hl_lv_t<newsize>('X');
    return hdtlib::hl_lv_t<newsize>(hdtlib::hl_lv_t<1>(arg)); // zero-extension
}

template <int newsize, int size>
hdtlib::hl_lv_t<newsize> hif_vhdl_conv_unsigned_signed(hdtlib::hl_lv_t<size> arg)
{
    if (!arg.is_01())
        return hdtlib::hl_lv_t<newsize>('X');
    return hif_vhdl_sxt<newsize>(arg);
}

template <int newsize, int size>
hdtlib::hl_lv_t<newsize> hif_vhdl_conv_unsigned_unsigned(hdtlib::hl_lv_t<size> arg)
{
    if (!arg.is_01())
        return hdtlib::hl_lv_t<newsize>('X');
    return hdtlib::hl_lv_t<newsize>(arg); // zero-extension
}

template <int newsize>
hdtlib::hl_lv_t<newsize> hif_vhdl_conv_unsigned(hdtlib::hl_logic_t arg)
{
    if (!arg.is_01())
        return hdtlib::hl_lv_t<newsize>('X');
    return hdtlib::hl_lv_t<newsize>(hdtlib::hl_lv_t<1>(arg)); // zero-extension
}

#endif // HIF2SCSUPPORT_USE_HDTLIB

} // namespace hif_vhdl_ieee_std_logic_arith

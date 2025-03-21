/// @file hif_vhdl_ieee_std_logic_arith.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif2scSupport/hif2scSupport/config.hpp"

namespace hif_vhdl_ieee_std_logic_arith
{

/// @name Type definitions.
/// @{

typedef int hif_vhdl_small_int;

/// @}

/// @name Relational operators
/// @{

template <int size> bool hif_vhdl__op_eq_signed(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2);

template <int size> bool hif_vhdl__op_neq_signed(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2);

template <typename T, size_t size> bool hif_vhdl__op_eq_signed(T (&v1)[size], T (&v2)[size]);

template <typename T, size_t size> bool hif_vhdl__op_neq_signed(T (&v1)[size], T (&v2)[size]);

template <int size> bool hif_vhdl__op_eq_unsigned(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2);

template <int size> bool hif_vhdl__op_neq_unsigned(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2);

template <typename T, size_t size> bool hif_vhdl__op_eq_unsigned(T (&v1)[size], T (&v2)[size]);

template <typename T, size_t size> bool hif_vhdl__op_neq_unsigned(T (&v1)[size], T (&v2)[size]);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size> bool hif_vhdl__op_eq_signed(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size> bool hif_vhdl__op_eq_signed(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2);

template <int size> bool hif_vhdl__op_eq_signed(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size> bool hif_vhdl__op_eq_signed(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2);

template <int size> bool hif_vhdl__op_neq_signed(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size> bool hif_vhdl__op_neq_signed(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2);

template <int size> bool hif_vhdl__op_neq_signed(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size> bool hif_vhdl__op_neq_signed(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2);

template <int size> bool hif_vhdl__op_eq_unsigned(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size> bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size> bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2);

template <int size> bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size> bool hif_vhdl__op_neq_unsigned(const hdtlib::hl_bv_t<size> &v1, const hdtlib::hl_bv_t<size> &v2);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

/// @name Arithmetic operators
/// @{

template <int size, template <int> class T> struct RetType {
    typedef sc_dt::sc_lv<size> Type;
    typedef sc_dt::sc_bigint<size> Int;
    typedef sc_dt::sc_biguint<size> UInt;
};

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size> struct RetType<size, hdtlib::hl_lv_t> {
    typedef hdtlib::hl_lv_t<size> Type;
    typedef hdtlib::hl_int_t<size> Int;
    typedef hdtlib::hl_uint_t<size> UInt;
};

template <int size> struct RetType<size, hdtlib::hl_bv_t> {
    typedef hdtlib::hl_lv_t<size> Type;
    typedef hdtlib::hl_int_t<size> Int;
    typedef hdtlib::hl_uint_t<size> UInt;
};

#endif

template <int size, template <int> class T1, template <int> class T2>
typename RetType<size, T1>::Type hif_vhdl__op_plus_signed(const T1<size> &v1, const T2<size> &v2);

template <int size, template <int> class T1, template <int> class T2>
typename RetType<size, T1>::Type hif_vhdl__op_minus_signed(const T1<size> &v1, const T2<size> &v2);

template <int size1, int size2, template <int> class T1, template <int> class T2>
typename RetType<size1 + size2, T1>::Type hif_vhdl__op_mult_signed(const T1<size1> &v1, const T2<size2> &v2);

template <int size, template <int> class T1, template <int> class T2>
typename RetType<size, T1>::Type hif_vhdl__op_plus_unsigned(const T1<size> &v1, const T2<size> &v2);

template <int size, template <int> class T1, template <int> class T2>
typename RetType<size, T1>::Type hif_vhdl__op_minus_unsigned(const T1<size> &v1, const T2<size> &v2);

template <int size1, int size2, template <int> class T1, template <int> class T2>
typename RetType<size1 + size2, T1>::Type hif_vhdl__op_mult_unsigned(const T1<size1> &v1, const T2<size2> &v2);

template <int size, template <int> class T1> typename RetType<size, T1>::Type hif_vhdl__op_abs(T1<size> arg);

/// @}

/// @name Sign-extension operators.
/// @{

template <int size2, int size1> sc_dt::sc_lv<size2> hif_vhdl_sxt(sc_dt::sc_lv<size1> arg);

template <int size2, int size1> sc_dt::sc_lv<size2> hif_vhdl_ext(sc_dt::sc_lv<size1> arg);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size2, int size1> hdtlib::hl_lv_t<size2> hif_vhdl_sxt(hdtlib::hl_lv_t<size1> arg);

template <int size2, int size1> hdtlib::hl_lv_t<size2> hif_vhdl_sxt(hdtlib::hl_bv_t<size1> arg);

template <int size2, int size1> hdtlib::hl_lv_t<size2> hif_vhdl_ext(hdtlib::hl_lv_t<size1> arg);

template <int size2, int size1> hdtlib::hl_lv_t<size2> hif_vhdl_ext(hdtlib::hl_bv_t<size1> arg);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

/// @name Shift operators.
/// @{

template <int size1, int size2> sc_dt::sc_lv<size1> hif_vhdl_shl(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count);

template <int size1, int size2>
sc_dt::sc_lv<size1> hif_vhdl_shr_signed(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count);

template <int size1, int size2>
sc_dt::sc_lv<size1> hif_vhdl_shr_unsigned(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shl(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count);

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shl(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_bv_t<size2> count);

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shr_signed(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count);

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shr_signed(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_bv_t<size2> count);

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shr_unsigned(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count);

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shr_unsigned(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_bv_t<size2> count);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

/// @name Conversion operators.
/// @{

template <int size> long long int hif_vhdl_conv_integer_signed(sc_dt::sc_lv<size> arg);

template <int size> long long int hif_vhdl_conv_integer_unsigned(sc_dt::sc_lv<size> arg);

template <int newsize, int size> sc_dt::sc_lv<newsize> hif_vhdl_conv_std_logic_vector_signed(sc_dt::sc_lv<size> arg);

template <int newsize, int size> sc_dt::sc_lv<newsize> hif_vhdl_conv_std_logic_vector_unsigned(sc_dt::sc_lv<size> arg);

template <int newsize> sc_dt::sc_lv<newsize> hif_vhdl_conv_std_logic_vector(sc_dt::sc_logic arg);

template <int newsize, int size> sc_dt::sc_lv<newsize> hif_vhdl_conv_signed_signed(sc_dt::sc_lv<size> arg);

template <int newsize, int size> sc_dt::sc_lv<newsize> hif_vhdl_conv_signed_unsigned(sc_dt::sc_lv<size> arg);

template <int newsize> sc_dt::sc_lv<newsize> hif_vhdl_conv_signed(sc_dt::sc_logic arg);

template <int newsize, int size> sc_dt::sc_lv<newsize> hif_vhdl_conv_unsigned_signed(sc_dt::sc_lv<size> arg);

template <int newsize, int size> sc_dt::sc_lv<newsize> hif_vhdl_conv_unsigned_unsigned(sc_dt::sc_lv<size> arg);

template <int newsize> sc_dt::sc_lv<newsize> hif_vhdl_conv_unsigned(sc_dt::sc_logic arg);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size> long long int hif_vhdl_conv_integer_signed(hdtlib::hl_lv_t<size> arg);

template <int size> long long int hif_vhdl_conv_integer_unsigned(hdtlib::hl_lv_t<size> arg);

template <int newsize, int size>
hdtlib::hl_lv_t<newsize> hif_vhdl_conv_std_logic_vector_signed(hdtlib::hl_lv_t<size> arg);

template <int newsize, int size>
hdtlib::hl_lv_t<newsize> hif_vhdl_conv_std_logic_vector_unsigned(hdtlib::hl_lv_t<size> arg);

template <int newsize> hdtlib::hl_lv_t<newsize> hif_vhdl_conv_std_logic_vector(hdtlib::hl_logic_t arg);

template <int newsize, int size> hdtlib::hl_lv_t<newsize> hif_vhdl_conv_signed_signed(hdtlib::hl_lv_t<size> arg);

template <int newsize, int size> hdtlib::hl_lv_t<newsize> hif_vhdl_conv_signed_unsigned(hdtlib::hl_lv_t<size> arg);

template <int newsize> hdtlib::hl_lv_t<newsize> hif_vhdl_conv_signed(hdtlib::hl_logic_t arg);

template <int newsize, int size> hdtlib::hl_lv_t<newsize> hif_vhdl_conv_unsigned_signed(hdtlib::hl_lv_t<size> arg);

template <int newsize, int size> hdtlib::hl_lv_t<newsize> hif_vhdl_conv_unsigned_unsigned(hdtlib::hl_lv_t<size> arg);

template <int newsize> hdtlib::hl_lv_t<newsize> hif_vhdl_conv_unsigned(hdtlib::hl_logic_t arg);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

} // namespace hif_vhdl_ieee_std_logic_arith

#include "hif2scSupport/hif_vhdl_ieee_std_logic_arith.i.hpp"

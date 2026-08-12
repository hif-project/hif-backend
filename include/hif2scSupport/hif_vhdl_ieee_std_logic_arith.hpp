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

using hif_vhdl_small_int = int;

/// @}

/// @name Relational operators
/// @{

template <int size> auto hif_vhdl__opp_eq_signed(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> bool;

template <int size> auto hif_vhdl__opp_neq_signed(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> bool;

template <typename T, size_t size> auto hif_vhdl__opp_eq_signed(T (&v1)[size], T (&v2)[size]) -> bool;

template <typename T, size_t size> auto hif_vhdl__opp_neq_signed(T (&v1)[size], T (&v2)[size]) -> bool;

template <int size> auto hif_vhdl__opp_eq_unsigned(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> bool;

template <int size> auto hif_vhdl__opp_neq_unsigned(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> bool;

template <typename T, size_t size> auto hif_vhdl__opp_eq_unsigned(T (&v1)[size], T (&v2)[size]) -> bool;

template <typename T, size_t size> auto hif_vhdl__opp_neq_unsigned(T (&v1)[size], T (&v2)[size]) -> bool;

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
    using Type = sc_dt::sc_lv<size>;
    using Int = sc_dt::sc_bigint<size>;
    using UInt = sc_dt::sc_biguint<size>;
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
auto hif_vhdl__opp_plus_signed(const T1<size> &v1, const T2<size> &v2) -> typename RetType<size, T1>::Type;

template <int size, template <int> class T1, template <int> class T2>
auto hif_vhdl__opp_minus_signed(const T1<size> &v1, const T2<size> &v2) -> typename RetType<size, T1>::Type;

template <int size1, int size2, template <int> class T1, template <int> class T2>
auto hif_vhdl__opp_mult_signed(const T1<size1> &v1, const T2<size2> &v2) -> typename RetType<size1 + size2, T1>::Type;

template <int size, template <int> class T1, template <int> class T2>
auto hif_vhdl__opp_plus_unsigned(const T1<size> &v1, const T2<size> &v2) -> typename RetType<size, T1>::Type;

template <int size, template <int> class T1, template <int> class T2>
auto hif_vhdl__opp_minus_unsigned(const T1<size> &v1, const T2<size> &v2) -> typename RetType<size, T1>::Type;

template <int size1, int size2, template <int> class T1, template <int> class T2>
auto hif_vhdl__opp_mult_unsigned(const T1<size1> &v1, const T2<size2> &v2) -> typename RetType<size1 + size2, T1>::Type;

template <int size, template <int> class T1> auto hif_vhdl__opp_abs(T1<size> arg) -> typename RetType<size, T1>::Type;

/// @}

/// @name Sign-extension operators.
/// @{

template <int size2, int size1> auto hif_vhdl_sxt(sc_dt::sc_lv<size1> arg) -> sc_dt::sc_lv<size2>;

template <int size2, int size1> auto hif_vhdl_ext(sc_dt::sc_lv<size1> arg) -> sc_dt::sc_lv<size2>;

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size2, int size1> hdtlib::hl_lv_t<size2> hif_vhdl_sxt(hdtlib::hl_lv_t<size1> arg);

template <int size2, int size1> hdtlib::hl_lv_t<size2> hif_vhdl_sxt(hdtlib::hl_bv_t<size1> arg);

template <int size2, int size1> hdtlib::hl_lv_t<size2> hif_vhdl_ext(hdtlib::hl_lv_t<size1> arg);

template <int size2, int size1> hdtlib::hl_lv_t<size2> hif_vhdl_ext(hdtlib::hl_bv_t<size1> arg);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

/// @name Shift operators.
/// @{

template <int size1, int size2> auto hif_vhdl_shl(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count) -> sc_dt::sc_lv<size1>;

template <int size1, int size2>
auto hif_vhdl_shr_signed(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count) -> sc_dt::sc_lv<size1>;

template <int size1, int size2>
auto hif_vhdl_shr_unsigned(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count) -> sc_dt::sc_lv<size1>;

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

template <int size> auto hif_vhdl_conv_integer_signed(sc_dt::sc_lv<size> arg) -> long long int;

template <int size> auto hif_vhdl_conv_integer_unsigned(sc_dt::sc_lv<size> arg) -> long long int;

template <int newsize, int size> auto hif_vhdl_conv_std_logic_vector_signed(sc_dt::sc_lv<size> arg) -> sc_dt::sc_lv<newsize>;

template <int newsize, int size> auto hif_vhdl_conv_std_logic_vector_unsigned(sc_dt::sc_lv<size> arg) -> sc_dt::sc_lv<newsize>;

template <int newsize> auto hif_vhdl_conv_std_logic_vector(sc_dt::sc_logic arg) -> sc_dt::sc_lv<newsize>;

template <int newsize, int size> auto hif_vhdl_conv_signed_signed(sc_dt::sc_lv<size> arg) -> sc_dt::sc_lv<newsize>;

template <int newsize, int size> auto hif_vhdl_conv_signed_unsigned(sc_dt::sc_lv<size> arg) -> sc_dt::sc_lv<newsize>;

template <int newsize> auto hif_vhdl_conv_signed(sc_dt::sc_logic arg) -> sc_dt::sc_lv<newsize>;

template <int newsize, int size> auto hif_vhdl_conv_unsigned_signed(sc_dt::sc_lv<size> arg) -> sc_dt::sc_lv<newsize>;

template <int newsize, int size> auto hif_vhdl_conv_unsigned_unsigned(sc_dt::sc_lv<size> arg) -> sc_dt::sc_lv<newsize>;

template <int newsize> auto hif_vhdl_conv_unsigned(sc_dt::sc_logic arg) -> sc_dt::sc_lv<newsize>;

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

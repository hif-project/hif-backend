/// @file hif_vhdl_ieee_std_logic_signed.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif2scSupport/hif2scSupport/config.hpp"

namespace hif_vhdl_ieee_std_logic_signed
{

/// @name Shift operators
/// @{

template <int size1, int size2> auto hif_vhdl_shl(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count) -> sc_dt::sc_lv<size1>;

template <int size1, int size2> auto hif_vhdl_shr(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count) -> sc_dt::sc_lv<size1>;

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shl(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count);

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shr(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

/// @name Conversion operators
/// @{

template <int size> auto hif_vhdl_conv_integer(sc_dt::sc_lv<size> arg) -> long long int;

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size> long long int hif_vhdl_conv_integer(hdtlib::hl_lv_t<size> arg);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

/// @name Relational operators
/// @{

template <int size> auto hif_vhdl__op_eq(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> bool;

template <int size> auto hif_vhdl__op_neq(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> bool;

template <int size> auto hif_vhdl__op_abs(sc_dt::sc_lv<size> arg) -> sc_dt::sc_lv<size>;

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size> bool hif_vhdl__op_eq(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size> bool hif_vhdl__op_neq(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size> hdtlib::hl_lv_t<size> hif_vhdl__op_abs(hdtlib::hl_lv_t<size> arg);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

/// @name Arithmetic operators
/// @{

template <int size> auto hif_vhdl__op_plus(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> sc_dt::sc_lv<size>;

template <int size> auto hif_vhdl__op_minus(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> sc_dt::sc_lv<size>;

template <int size1, int size2>
auto hif_vhdl__op_mult(const sc_dt::sc_lv<size1> &v1, const sc_dt::sc_lv<size2> &v2) -> sc_dt::sc_lv<size1 + size2>;

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size>
hdtlib::hl_lv_t<size> hif_vhdl__op_plus(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size>
hdtlib::hl_lv_t<size> hif_vhdl__op_minus(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size1, int size2>
hdtlib::hl_lv_t<size1 + size2> hif_vhdl__op_mult(const hdtlib::hl_lv_t<size1> &v1, const hdtlib::hl_lv_t<size2> &v2);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

} // namespace hif_vhdl_ieee_std_logic_signed

#include "hif2scSupport/hif_vhdl_ieee_std_logic_signed.i.hpp"

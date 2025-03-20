/// @file hif_vhdl_ieee_std_logic_unsigned.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2scSupport/hif2scSupport/config.hpp"

namespace hif_vhdl_ieee_std_logic_unsigned
{

/// @name Shift operators
/// @{

template <int size1, int size2>
sc_dt::sc_lv<size1> hif_vhdl_shl(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count);

template <int size1, int size2>
sc_dt::sc_lv<size1> hif_vhdl_shr(sc_dt::sc_lv<size1> arg, sc_dt::sc_lv<size2> count);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shl(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count);

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_vhdl_shr(hdtlib::hl_lv_t<size1> arg, hdtlib::hl_lv_t<size2> count);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

/// @name Conversion operators
/// @{

template <int size>
long long int hif_vhdl_conv_integer(sc_dt::sc_lv<size> arg);

/// @}

/// @name Relational operators
/// @{

template <int size>
bool hif_vhdl__op_eq(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2);

template <int size>
bool hif_vhdl__op_neq(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size>
bool hif_vhdl__op_eq(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size>
bool hif_vhdl__op_neq(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

/// @name Arithmetic operators
/// @{

template <int size>
sc_dt::sc_lv<size> hif_vhdl__op_plus(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2);

template <int size>
sc_dt::sc_lv<size> hif_vhdl__op_minus(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2);

template <int size1, int size2>
sc_dt::sc_lv<size1 + size2> hif_vhdl__op_mult(const sc_dt::sc_lv<size1> &v1, const sc_dt::sc_lv<size2> &v2);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size>
hdtlib::hl_lv_t<size> hif_vhdl__op_plus(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size>
hdtlib::hl_lv_t<size> hif_vhdl__op_minus(const hdtlib::hl_lv_t<size> &v1, const hdtlib::hl_lv_t<size> &v2);

template <int size1, int size2>
hdtlib::hl_lv_t<size1 + size2> hif_vhdl__op_mult(const hdtlib::hl_lv_t<size1> &v1, const hdtlib::hl_lv_t<size2> &v2);

#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @}

} // namespace hif_vhdl_ieee_std_logic_unsigned

#include "hif2scSupport/hif_vhdl_ieee_std_logic_unsigned.i.hpp"

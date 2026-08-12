/// @file hif_vhdl_ieee_numeric_std.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif2scSupport/hif2scSupport/config.hpp"

namespace hif_vhdl_ieee_numeric_std
{

/// @name Conversion operators
/// @{

HIF2SCSUPPORT_EXPORT
auto hif_vhdl_to_integer_signed(const sc_dt::sc_lv_base &param1) -> int32_t;

HIF2SCSUPPORT_EXPORT
auto hif_vhdl_to_integer_unsigned(const sc_dt::sc_lv_base &param1) -> uint32_t;

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int W> int32_t hif_vhdl_to_integer_signed(const hdtlib::hl_lv_t<W> &param1);

template <int W> uint32_t hif_vhdl_to_integer_unsigned(const hdtlib::hl_lv_t<W> &param1);

#endif

/// @}

/// @name Support methods.
/// @{

HIF2SCSUPPORT_EXPORT
auto hif_vhdl_std_match(const sc_dt::sc_logic &param1, const sc_dt::sc_logic &param2) -> bool;

HIF2SCSUPPORT_EXPORT
auto hif_vhdl_std_match(const sc_dt::sc_lv_base &param1, const sc_dt::sc_lv_base &param2) -> bool;

#ifdef HIF2SCSUPPORT_USE_HDTLIB

HIF2SCSUPPORT_EXPORT
bool hif_vhdl_std_match(const hdtlib::hl_logic_t &param1, const hdtlib::hl_logic_t &param2);

template <int W> bool hif_vhdl_std_match(const hdtlib::hl_lv_t<W> &param1, const hdtlib::hl_lv_t<W> &param2);

#endif

/// @}

/// @name Arithmetic operators.
/// @{

template <int size> auto hif_vhdl__op_abs(sc_dt::sc_lv<size> arg) -> sc_dt::sc_lv<size>;

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size> hdtlib::hl_lv_t<size> hif_vhdl__op_abs(hdtlib::hl_lv_t<size> arg);

#endif

/// @}

/// @name Relational operators
/// @{

template <int size> auto hif_vhdl__op_eq_signed(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> bool;

template <int size> auto hif_vhdl__op_neq_signed(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> bool;

template <typename T, size_t size> auto hif_vhdl__op_eq_signed(T (&v1)[size], T (&v2)[size]) -> bool;

template <typename T, size_t size> auto hif_vhdl__op_neq_signed(T (&v1)[size], T (&v2)[size]) -> bool;

template <int size> auto hif_vhdl__op_eq_unsigned(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> bool;

template <int size> auto hif_vhdl__op_neq_unsigned(const sc_dt::sc_lv<size> &v1, const sc_dt::sc_lv<size> &v2) -> bool;

template <typename T, size_t size> auto hif_vhdl__op_eq_unsigned(T (&v1)[size], T (&v2)[size]) -> bool;

template <typename T, size_t size> auto hif_vhdl__op_neq_unsigned(T (&v1)[size], T (&v2)[size]) -> bool;

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

} // namespace hif_vhdl_ieee_numeric_std

#include "hif2scSupport/hif_vhdl_ieee_numeric_std.i.hpp"

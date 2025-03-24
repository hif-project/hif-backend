/// @file hif_vhdl_ieee_std_logic_1164.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif2scSupport/hif2scSupport/config.hpp"

namespace hif_vhdl_ieee_std_logic_1164
{

#ifndef HIF2SCSUPPORT_USE_HDTLIB
using hif_vhdl_x01 = sc_dt::sc_logic;
using hif_vhdl_x01z = sc_dt::sc_logic;
using hif_vhdl_ux01 = sc_dt::sc_logic;
using hif_vhdl_ux01z = sc_dt::sc_logic;
#else
typedef hdtlib::hl_logic_t hif_vhdl_x01;
typedef hdtlib::hl_logic_t hif_vhdl_x01z;
typedef hdtlib::hl_logic_t hif_vhdl_ux01;
typedef hdtlib::hl_logic_t hif_vhdl_ux01z;
#endif

HIF2SCSUPPORT_EXPORT
auto hif_vhdl_resolved(const sc_dt::sc_lv_base &s) -> sc_dt::sc_logic;

HIF2SCSUPPORT_EXPORT
auto hif_vhdl_to_bit(const sc_dt::sc_logic &s, bool xmap = false) -> bool;

template <int size> auto hif_vhdl_to_bitvector(const sc_dt::sc_lv<size> &s, bool xmap = false) -> sc_dt::sc_bv<size>;

template <int size> auto hif_vhdl_to_x01(const sc_dt::sc_lv<size> &s) -> sc_dt::sc_lv<size>;

HIF2SCSUPPORT_EXPORT
auto hif_vhdl_to_x01(const sc_dt::sc_logic &s) -> sc_dt::sc_logic;

template <int size> auto hif_vhdl_to_ux01(const sc_dt::sc_lv<size> &s) -> sc_dt::sc_lv<size>;

HIF2SCSUPPORT_EXPORT
auto hif_vhdl_to_ux01(const sc_dt::sc_logic &s) -> sc_dt::sc_logic;

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int size> hdtlib::hl_logic_t hif_vhdl_resolved(const hdtlib::hl_lv_t<size> &s);

HIF2SCSUPPORT_EXPORT
bool hif_vhdl_to_bit(const hdtlib::hl_logic_t s, bool xmap = false);

template <int size>
hdtlib::hl_bv_t<size> hif_vhdl_to_bitvector(const hdtlib::hl_lv_t<size> &s, bool xmap = false);

template <int size> hdtlib::hl_lv_t<size> hif_vhdl_to_x01(const hdtlib::hl_lv_t<size> &s);

HIF2SCSUPPORT_EXPORT
hdtlib::hl_logic_t hif_vhdl_to_x01(const hdtlib::hl_logic_t &s);

template <int size> hdtlib::hl_lv_t<size> hif_vhdl_to_ux01(const hdtlib::hl_lv_t<size> &s);

HIF2SCSUPPORT_EXPORT
hdtlib::hl_logic_t hif_vhdl_to_ux01(const hdtlib::hl_logic_t &s);
#endif

//// FUNCTION rising_edge  (SIGNAL s : std_ulogic) RETURN BOOLEAN;
//ld->declarations.push_back(_makeAttribute("rising_edge", factory.boolean(), hif::copy(std_ulogic), factory.noValue(), false, hifFormat));
//// FUNCTION falling_edge (SIGNAL s : std_ulogic) RETURN BOOLEAN;
//ld->declarations.push_back(_makeAttribute("falling_edge", factory.boolean(), hif::copy(std_ulogic), factory.noValue(), false, hifFormat));

HIF2SCSUPPORT_EXPORT
auto hif_vhdl_is_x(const sc_dt::sc_logic &s) -> bool;

HIF2SCSUPPORT_EXPORT
auto hif_vhdl_is_x(const sc_dt::sc_lv_base &s) -> bool;

#ifdef HIF2SCSUPPORT_USE_HDTLIB
HIF2SCSUPPORT_EXPORT
bool hif_vhdl_is_x(const hdtlib::hl_logic_t s);

template <int size> bool hif_vhdl_is_x(const hdtlib::hl_lv_t<size> s);
#endif

} // namespace hif_vhdl_ieee_std_logic_1164

#include "hif2scSupport/hif_vhdl_ieee_std_logic_1164.i.hpp"

/// @file hif_vhdl_ieee_std_logic_textio.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif2scSupport/hif2scSupport/config.hpp"
#include "hif_vhdl_std_textio.hpp"
#include <systemc>

namespace hif_vhdl_ieee_std_logic_textio
{

// -- Read and Write procedures for STD_ULOGIC and STD_ULOGIC_VECTOR
// -- Read and Write procedures for STD_LOGIC_VECTOR

// procedure READ(L:inout LINE; VALUE:out STD_ULOGIC);
// procedure READ(L:inout LINE; VALUE:out STD_ULOGIC; GOOD: out BOOLEAN);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_logic &value, bool &good);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_logic &value);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, hdtlib::hl_logic_t &value, bool &good);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, hdtlib::hl_logic_t &value);
#endif

// procedure READ(L:inout LINE; VALUE:out STD_ULOGIC_VECTOR);
// procedure READ(L:inout LINE; VALUE:out STD_ULOGIC_VECTOR; GOOD: out BOOLEAN);
// procedure READ(L:inout LINE; VALUE:out STD_LOGIC_VECTOR);
// procedure READ(L:inout LINE; VALUE:out STD_LOGIC_VECTOR; GOOD: out BOOLEAN);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value, bool &good);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int W>
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, hdtlib::hl_lv_t<W> &value, bool &good);

template <int W>
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, hdtlib::hl_lv_t<W> &value);
#endif

HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value);

// procedure WRITE(L:inout LINE; VALUE:in STD_ULOGIC;
//                 JUSTIFIED:in SIDE := RIGHT; FIELD:in WIDTH := 0);
// procedure WRITE(L:inout LINE; VALUE:in STD_ULOGIC_VECTOR;
//                 JUSTIFIED:in SIDE := RIGHT; FIELD:in WIDTH := 0);
// procedure WRITE(L:inout LINE; VALUE:in STD_LOGIC_VECTOR;
//                 JUSTIFIED:in SIDE := RIGHT; FIELD:in WIDTH := 0);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    sc_dt::sc_logic value,
    hif_vhdl_std_textio::hif_vhdl_side justified = hif_vhdl_std_textio::hif_vhdl_right,
    hif_vhdl_std_textio::hif_vhdl_width field    = 0);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    sc_dt::sc_lv_base value,
    hif_vhdl_std_textio::hif_vhdl_side justified = hif_vhdl_std_textio::hif_vhdl_right,
    hif_vhdl_std_textio::hif_vhdl_width field    = 0);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    hdtlib::hl_logic_t value,
    hif_vhdl_std_textio::hif_vhdl_side justified = hif_vhdl_std_textio::hif_vhdl_right,
    hif_vhdl_std_textio::hif_vhdl_width field    = 0);

template <int W>
void hif_vhdl_write(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    hdtlib::hl_lv_t<W> value,
    hif_vhdl_std_textio::hif_vhdl_side justified = hif_vhdl_std_textio::hif_vhdl_right,
    hif_vhdl_std_textio::hif_vhdl_width field    = 0);

#endif

// --
// -- Read and Write procedures for Hex and Octal values.
// -- The values appear in the file as a series of characters
// -- between 0-F (Hex), or 0-7 (Octal) respectively.
// --

// -- Hex
// procedure HREAD(L:inout LINE; VALUE:out STD_ULOGIC_VECTOR);
// procedure HREAD(L:inout LINE; VALUE:out STD_LOGIC_VECTOR);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value, bool &good);

// procedure HREAD(L:inout LINE; VALUE:out STD_ULOGIC_VECTOR; GOOD: out BOOLEAN);
// procedure HREAD(L:inout LINE; VALUE:out STD_LOGIC_VECTOR; GOOD: out BOOLEAN);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int W>
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, hdtlib::hl_lv_t<W> &value, bool &good);

template <int W>
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, hdtlib::hl_lv_t<W> &value);
#endif

HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value);

// procedure HWRITE(L:inout LINE; VALUE:in STD_ULOGIC_VECTOR;
//                  JUSTIFIED:in SIDE := RIGHT; FIELD:in WIDTH := 0);
// procedure HWRITE(L:inout LINE; VALUE:in STD_LOGIC_VECTOR;
//                  JUSTIFIED:in SIDE := RIGHT; FIELD:in WIDTH := 0);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_hwrite(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    sc_dt::sc_lv_base value,
    hif_vhdl_std_textio::hif_vhdl_side justified = hif_vhdl_std_textio::hif_vhdl_right,
    hif_vhdl_std_textio::hif_vhdl_width field    = 0);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int W>
void hif_vhdl_hwrite(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    hdtlib::hl_lv_t<W> value,
    hif_vhdl_std_textio::hif_vhdl_side justified = hif_vhdl_std_textio::hif_vhdl_right,
    hif_vhdl_std_textio::hif_vhdl_width field    = 0);

#endif

// -- Octal

// procedure OREAD(L:inout LINE; VALUE:out STD_ULOGIC_VECTOR; GOOD: out BOOLEAN);
// procedure OREAD(L:inout LINE; VALUE:out STD_LOGIC_VECTOR; GOOD: out BOOLEAN);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value, bool &good);

// procedure OREAD(L:inout LINE; VALUE:out STD_ULOGIC_VECTOR);
// procedure OREAD(L:inout LINE; VALUE:out STD_LOGIC_VECTOR);

HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value);

// procedure OWRITE(L:inout LINE; VALUE:in STD_ULOGIC_VECTOR;
//                  JUSTIFIED:in SIDE := RIGHT; FIELD:in WIDTH := 0);
// procedure OWRITE(L:inout LINE; VALUE:in STD_LOGIC_VECTOR;
//                  JUSTIFIED:in SIDE := RIGHT; FIELD:in WIDTH := 0);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int W>
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, hdtlib::hl_lv_t<W> &value, bool &good);

template <int W>
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, hdtlib::hl_lv_t<W> &value);
#endif

HIF2SCSUPPORT_EXPORT
void hif_vhdl_owrite(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    sc_dt::sc_lv_base value,
    hif_vhdl_std_textio::hif_vhdl_side justified = hif_vhdl_std_textio::hif_vhdl_right,
    hif_vhdl_std_textio::hif_vhdl_width field    = 0);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int W>
void hif_vhdl_owrite(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    hdtlib::hl_lv_t<W> value,
    hif_vhdl_std_textio::hif_vhdl_side justified = hif_vhdl_std_textio::hif_vhdl_right,
    hif_vhdl_std_textio::hif_vhdl_width field    = 0);

#endif

HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value, bool &good);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value);

} // namespace hif_vhdl_ieee_std_logic_textio

#include "hif2scSupport/hif_vhdl_ieee_std_logic_textio.i.hpp"

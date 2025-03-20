/// @file hif_vhdl_std_textio.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif2scSupport/hif2scSupport/config.hpp"
#include "hif_vhdl_standard.hpp"
#include <stdint.h>
#include <systemc>

namespace hif_vhdl_std_textio
{

// type LINE is access STRING;-- a line is a pointer to a STRING value
typedef std::string *hif_vhdl_line;

//  type TEXT is file of STRING;-- a file of variable-length ASCII records
typedef FILE *hif_vhdl_text;

//  type SIDE is (RIGHT, LEFT);-- for justifying output data within fields
enum hif_vhdl_side { hif_vhdl_right, hif_vhdl_left };

//  subtype WIDTH is NATURAL;-- for specifying widths of output fields
typedef uint32_t hif_vhdl_width;

//  file INPUT: TEXT open READ_MODE is "STD_INPUT";
HIF2SCSUPPORT_EXPORT
extern hif_vhdl_text hif_vhdl_input;

//  file OUTPUT: TEXT open WRITE_MODE is "STD_OUTPUT";
HIF2SCSUPPORT_EXPORT
extern hif_vhdl_text hif_vhdl_output;

// Routines to support standard statements.

HIF2SCSUPPORT_EXPORT
hif_vhdl_text hif_vhdl_file_open(
    std::string external_name,
    hif_vhdl_standard::hif_vhdl_file_open_kind open_kind = hif_vhdl_standard::hif_vhdl_read_mode);

//  -- Input Routines for Standard Types

// procedure FILE_OPEN (file F: TEXT; External_Name: in STRING;
// Open_Kind: in FILE_OPEN_KIND := READ_MODE);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_file_open(
    hif_vhdl_text &f,
    std::string external_name,
    hif_vhdl_standard::hif_vhdl_file_open_kind open_kind = hif_vhdl_standard::hif_vhdl_read_mode);

// procedure FILE_OPEN (Status: out FILE_OPEN_STATUS; file F: TEXT;
// External_Name: in STRING; Open_Kind: in FILE_OPEN_KIND := READ_MODE);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_file_open(
    hif_vhdl_standard::hif_vhdl_file_open_status &status,
    hif_vhdl_text &f,
    std::string external_name,
    hif_vhdl_standard::hif_vhdl_file_open_kind open_kind = hif_vhdl_standard::hif_vhdl_read_mode);

// procedure FILE_CLOSE (file F: TEXT);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_file_close(hif_vhdl_text &f);

// procedure READ (file F: TEXT; VALUE: out STRING);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_text &f, std::string &value);

// procedure WRITE (file F: TEXT; VALUE: in STRING);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(hif_vhdl_text &f, std::string value);

//  procedure READLINE (file F: TEXT; L: inout LINE);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_readline(hif_vhdl_text &f, hif_vhdl_line &l);

//  procedure READ (L: inout LINE; VALUE: out BIT; GOOD: out BOOLEAN);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, sc_dt::sc_bit &value, bool &good);

//  procedure READ (L: inout LINE; VALUE: out BIT);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, sc_dt::sc_bit &value);

//  procedure READ (L: inout LINE; VALUE: out BIT_VECTOR; GOOD: out BOOLEAN);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, sc_dt::sc_bv_base &value, bool &good);

//  procedure READ (L: inout LINE; VALUE: out BIT_VECTOR);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, sc_dt::sc_bv_base &value);

//  procedure READ (L: inout LINE; VALUE: out BOOLEAN; GOOD: out BOOLEAN);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, bool &value, bool &good);

//  procedure READ (L: inout LINE; VALUE: out BOOLEAN);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, bool &value);

//  procedure READ (L: inout LINE; VALUE: out CHARACTER; GOOD: out BOOLEAN);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, char &value, bool &good);

//  procedure READ (L: inout LINE; VALUE: out CHARACTER);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, char &value);

//  procedure READ (L: inout LINE; VALUE: out INTEGER; GOOD: out BOOLEAN);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, int32_t &value, bool &good);

//  procedure READ (L: inout LINE; VALUE: out INTEGER);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, int32_t &value);

//  procedure READ (L: inout LINE; VALUE: out REAL; GOOD: out BOOLEAN);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, double &value, bool &good);

//  procedure READ (L: inout LINE; VALUE: out REAL);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, double &value);

//  procedure READ (L: inout LINE; VALUE: out STRING; GOOD: out BOOLEAN);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, std::string &value, bool &good);

//  procedure READ (L: inout LINE; VALUE: out STRING);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, std::string &value);

//  procedure READ (L: inout LINE; VALUE: out TIME; GOOD: out BOOLEAN);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, sc_core::sc_time &value, bool &good);

//  procedure READ (L: inout LINE; VALUE: out TIME);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_read(hif_vhdl_line &l, sc_core::sc_time &value);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

//  procedure READ (L: inout LINE; VALUE: out BIT_VECTOR; GOOD: out BOOLEAN);
template <int W>
void hif_vhdl_read(hif_vhdl_line &l, hdtlib::hl_bv_t<W> &value, bool &good);

//  procedure READ (L: inout LINE; VALUE: out BIT_VECTOR);
template <int W>
void hif_vhdl_read(hif_vhdl_line &l, hdtlib::hl_bv_t<W> &value);

#endif

//  -- Output Routines for Standard Types

//  procedure WRITELINE (file F: TEXT; L: inout LINE);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_writeline(hif_vhdl_text &f, hif_vhdl_line &l);

//  procedure WRITE (L: inout LINE; VALUE: in BIT;
//                   JUSTIFIED: in SIDE := RIGHT; FIELD: in WIDTH := 0);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(
    hif_vhdl_line &l,
    sc_dt::sc_bit value,
    hif_vhdl_side justified = hif_vhdl_right,
    hif_vhdl_width field    = 0);

//  procedure WRITE (L: inout LINE; VALUE: in BIT_VECTOR;
//                   JUSTIFIED: in SIDE := RIGHT; FIELD: in WIDTH := 0);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(
    hif_vhdl_line &l,
    sc_dt::sc_bv_base value,
    hif_vhdl_side justified = hif_vhdl_right,
    hif_vhdl_width field    = 0);

//  procedure WRITE (L: inout LINE; VALUE: in BOOLEAN;
//                   JUSTIFIED: in SIDE := RIGHT; FIELD: in WIDTH := 0);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(hif_vhdl_line &l, bool value, hif_vhdl_side justified = hif_vhdl_right, hif_vhdl_width field = 0);

//  procedure WRITE (L: inout LINE; VALUE: in CHARACTER;
//                   JUSTIFIED: in SIDE := RIGHT; FIELD: in WIDTH := 0);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(hif_vhdl_line &l, char value, hif_vhdl_side justified = hif_vhdl_right, hif_vhdl_width field = 0);

//  procedure WRITE (L: inout LINE; VALUE: in INTEGER;
//                   JUSTIFIED: in SIDE := RIGHT; FIELD: in WIDTH := 0);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(
    hif_vhdl_line &l,
    int32_t value,
    hif_vhdl_side justified = hif_vhdl_right,
    hif_vhdl_width field    = 0);

//  procedure WRITE (L: inout LINE; VALUE: in REAL;
//                   JUSTIFIED: in SIDE := RIGHT; FIELD: in WIDTH := 0;
//                   DIGITS: in NATURAL := 0);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(
    hif_vhdl_line &l,
    int32_t value,
    hif_vhdl_side justified = hif_vhdl_right,
    hif_vhdl_width field    = 0,
    uint32_t digits         = 0);

//  procedure WRITE (L: inout LINE; VALUE: in STRING;
//                   JUSTIFIED: in SIDE := RIGHT; FIELD: in WIDTH := 0);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(
    hif_vhdl_line &l,
    std::string value,
    hif_vhdl_side justified = hif_vhdl_right,
    hif_vhdl_width field    = 0);

//  procedure WRITE (L: inout LINE; VALUE: in TIME;
//                   JUSTIFIED: in SIDE := RIGHT; FIELD: in WIDTH := 0;
//                   UNIT: in TIME := ns);
HIF2SCSUPPORT_EXPORT
void hif_vhdl_write(
    hif_vhdl_line &l,
    sc_core::sc_time value,
    hif_vhdl_side justified    = hif_vhdl_right,
    hif_vhdl_width field       = 0,
    sc_core::sc_time_unit unit = sc_core::SC_NS);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int W>
void hif_vhdl_write(
    hif_vhdl_line &l,
    hdtlib::hl_bv_t<W> value,
    hif_vhdl_side justified = hif_vhdl_right,
    hif_vhdl_width field    = 0);

#endif

//  -- File Position Predicates

// function ENDFILE (F: in TEXT) return BOOLEAN;
HIF2SCSUPPORT_EXPORT
bool hif_vhdl_endfile(hif_vhdl_text f);

} // namespace hif_vhdl_std_textio

#include "hif2scSupport/hif_vhdl_std_textio.i.hpp"

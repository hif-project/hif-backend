/// @file hif_vhdl_standard.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <string>
#include <systemc>

#include "hif2scSupport/hif2scSupport/config.hpp"

namespace hif_vhdl_standard
{

// ///////////////////////////////////////////////////////////////////
// Types
// ///////////////////////////////////////////////////////////////////

enum hif_vhdl_severity_level { hif_vhdl_note, hif_vhdl_warning, hif_vhdl_error, hif_vhdl_failure };

// hif_vhdl_time_units --> sc_core::sc_time_unit

typedef sc_core::sc_time hif_vhdl_delay_length;

enum hif_vhdl_file_open_kind { hif_vhdl_read_mode, hif_vhdl_write_mode, hif_vhdl_append_mode };

enum hif_vhdl_file_open_status { hif_vhdl_open_ok, hif_vhdl_status_error, hif_vhdl_name_error, hif_vhdl_mode_error };

// ///////////////////////////////////////////////////////////////////
// Methods
// ///////////////////////////////////////////////////////////////////

HIF2SCSUPPORT_EXPORT
void hif_vhdl_assert(
    const bool condition,
    const std::string &report           = std::string(),
    const hif_vhdl_severity_level level = hif_vhdl_error);

HIF2SCSUPPORT_EXPORT
long long int hif_vhdl_castRealToInt(double param, int size, bool sign);

// hif_vhdl_now() --> sc_core::sc_time_stamp()

} // namespace hif_vhdl_standard

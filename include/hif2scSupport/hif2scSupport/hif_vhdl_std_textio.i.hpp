/// @file hif_vhdl_std_textio.i.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "../hif_vhdl_std_textio.hpp"

namespace hif_vhdl_std_textio
{

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int W> void hif_vhdl_read(hif_vhdl_line &l, hdtlib::hl_bv_t<W> &value, bool &good)
{
    sc_dt::sc_bv<W> tmp = value.to_string().c_str();
    hif_vhdl_read(l, tmp, good);
    value = tmp.to_string().c_str();
}

template <int W> void hif_vhdl_read(hif_vhdl_line &l, hdtlib::hl_bv_t<W> &value)
{
    bool good;
    hif_vhdl_read(l, value, good);
}

template <int W>
void hif_vhdl_write(hif_vhdl_line &l, hdtlib::hl_bv_t<W> value, hif_vhdl_side justified, hif_vhdl_width field)
{
    sc_dt::sc_bv<W> tmp = value.to_string().c_str();
    hif_vhdl_write(l, tmp, justified, field);
}

#endif

} // namespace hif_vhdl_std_textio

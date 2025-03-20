/// @file hif_assign.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2scSupport/hif2scSupport/hif_assign.hpp"

namespace hif_systemc_extensions
{

sc_dt::sc_logic_value_t to_bit(sc_dt::sc_logic v) { return v.value(); }

sc_dt::sc_logic_value_t to_bit(sc_dt::sc_bit v) { return v.to_bool() ? sc_dt::Log_1 : sc_dt::Log_0; }

bool to_bit(bool v) { return v; }

const sc_dt::sc_bv<8> _hif_assign_getValue(const uint8_t &ret)
{
    return sc_dt::sc_bv<8>(static_cast<unsigned int>(ret));
}

const sc_dt::sc_bv<16> _hif_assign_getValue(const uint16_t &ret)
{
    return sc_dt::sc_bv<16>(static_cast<unsigned int>(ret));
}

const sc_dt::sc_bv<32> _hif_assign_getValue(const uint32_t &ret) { return sc_dt::sc_bv<32>(ret); }

const sc_dt::sc_bv<64> _hif_assign_getValue(const uint64_t &ret) { return sc_dt::sc_bv<64>(ret); }

const sc_dt::sc_bv<8> _hif_assign_getValue(const int8_t &ret) { return sc_dt::sc_bv<8>(ret); }

const sc_dt::sc_bv<16> _hif_assign_getValue(const int16_t &ret) { return sc_dt::sc_bv<16>(ret); }

const sc_dt::sc_bv<32> _hif_assign_getValue(const int32_t &ret) { return sc_dt::sc_bv<32>(ret); }

const sc_dt::sc_bv<64> _hif_assign_getValue(const int64_t &ret) { return sc_dt::sc_bv<64>(ret); }

#ifdef HIF2SCSUPPORT_USE_HDTLIB
hdtlib::hl_logic_t to_bit(hdtlib::hl_logic_t v) { return v; }
#endif

} // namespace hif_systemc_extensions

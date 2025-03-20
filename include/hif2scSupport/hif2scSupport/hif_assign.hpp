/// @file hif_assign.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "config.hpp"

/////////////////////////////////////////
// Declarations
/////////////////////////////////////////

namespace hif_systemc_extensions
{

// /////////////////////////////////////////////////////////////////////////
// Support methods for hif_assign.
// /////////////////////////////////////////////////////////////////////////

HIF2SCSUPPORT_EXPORT
sc_dt::sc_logic_value_t to_bit(sc_dt::sc_logic v);

HIF2SCSUPPORT_EXPORT
sc_dt::sc_logic_value_t to_bit(sc_dt::sc_bit v);

HIF2SCSUPPORT_EXPORT
bool to_bit(bool v);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
HIF2SCSUPPORT_EXPORT
hdtlib::hl_logic_t to_bit(hdtlib::hl_logic_t v);
#endif // HIF2SCSUPPORT_USE_HDTLIB

/// @name This set of methods is intended as a way to assign a <N>-dimensional
/// source to a <M>-dimensional target, where source and target could have
/// SystemC types or native types.
/// It also allow to assign a specific portion of the source or target,
/// depending on which one is defined as multi-dimensional object.
///
/// @n All methods shown the following form:
/// @begin code
/// template< typename T, typename S >
/// void hif_assign( T& target, S source,
///        unsigned int size, unsigned int left, unsigned int right );
/// @end code
///
/// Where T and S indicate generic types for the target and the source.
/// @param target represents the target.
/// @param source represents the source.
/// @param size indicates the size of the multi-dimensional parameter (or   // TODO check
/// both, since they must be equal wrt to HIF assignment assumptions).
/// @param left indicates the left bound of the portion to be assigned (if any).
/// @param right indicates the right bound of the portion to be assigned (if any).
///
//@{

// --------------------------------------------------------

template <typename T, typename S>
void hif_assign(
    T &target,
    const S &source,
    unsigned int size,
    unsigned int left1  = static_cast<unsigned int>(-1),
    unsigned int right1 = static_cast<unsigned int>(-1),
    unsigned int left2  = static_cast<unsigned int>(-1),
    unsigned int right2 = static_cast<unsigned int>(-1));

template <typename T, typename S>
void hif_assign(
    sc_core::sc_signal<T> &target,
    const S &source,
    unsigned int size,
    unsigned int left1  = static_cast<unsigned int>(-1),
    unsigned int right1 = static_cast<unsigned int>(-1),
    unsigned int left2  = static_cast<unsigned int>(-1),
    unsigned int right2 = static_cast<unsigned int>(-1));

template <typename T, typename S>
void hif_assign(
    sc_core::sc_out<T> &target,
    const S &source,
    unsigned int size,
    unsigned int left1  = static_cast<unsigned int>(-1),
    unsigned int right1 = static_cast<unsigned int>(-1),
    unsigned int left2  = static_cast<unsigned int>(-1),
    unsigned int right2 = static_cast<unsigned int>(-1));

template <typename T, typename S>
void hif_assign(
    sc_core::sc_inout<T> &target,
    const S &source,
    unsigned int size,
    unsigned int left1  = static_cast<unsigned int>(-1),
    unsigned int right1 = static_cast<unsigned int>(-1),
    unsigned int left2  = static_cast<unsigned int>(-1),
    unsigned int right2 = static_cast<unsigned int>(-1));

template <typename T, typename S>
void hif_assign(
    sc_core::sc_vector<T> &target,
    const S &source,
    unsigned int size,
    unsigned int left1  = static_cast<unsigned int>(-1),
    unsigned int right1 = static_cast<unsigned int>(-1),
    unsigned int left2  = static_cast<unsigned int>(-1),
    unsigned int right2 = static_cast<unsigned int>(-1));

template <typename T, typename S, size_t as>
void hif_assign(
    T (&target)[as],
    const S &source,
    size_t size,
    size_t left1  = static_cast<size_t>(-1),
    size_t right1 = static_cast<size_t>(-1),
    size_t left2  = static_cast<size_t>(-1),
    size_t right2 = static_cast<size_t>(-1));

template <typename T, typename S, size_t as1, size_t as2>
void hif_assign(
    T (&target)[as1][as2],
    const S &source,
    size_t size,
    size_t left1  = static_cast<size_t>(-1),
    size_t right1 = static_cast<size_t>(-1),
    size_t left2  = static_cast<size_t>(-1),
    size_t right2 = static_cast<size_t>(-1));

} // namespace hif_systemc_extensions

/////////////////////////////////////////
// Template-implementation header
/////////////////////////////////////////

#include "hif_assign.i.hpp"

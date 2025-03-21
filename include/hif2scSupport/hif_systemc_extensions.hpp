/// @file hif_systemc_extensions.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <string>
#include <systemc>
#include <type_traits>

#include "hif2scSupport/hif2scSupport/config.hpp"
#include "hif2scSupport/hif_after.hpp"
#include "hif2scSupport/hif_assign.hpp"
#include "hif2scSupport/hif_vector_slice.hpp"

namespace hif_systemc_extensions
{

/// @brief Tolerance for real comparison
HIF2SCSUPPORT_EXPORT
extern double hif_dTolerance;

/// @brief Tolerance for real comparison
HIF2SCSUPPORT_EXPORT
extern float hif_fTolerance;

/// @brief Tolerance for real comparison
HIF2SCSUPPORT_EXPORT
extern long double hif_ldTolerance;

/// @brief Sets the resource path
HIF2SCSUPPORT_EXPORT
void hif_setResourcePath(const std::string &path);
/// @brief Gets the resource path
HIF2SCSUPPORT_EXPORT
std::string hif_getResourcePath();
/// @brief Gets the resource file name.
HIF2SCSUPPORT_EXPORT
std::string hif_getResourceFileName(const std::string &name);

/// @brief Compares two reals in a safe way.
/// @param v1 The first operand.
/// @param v2 The second operand.
/// @return True if equals
HIF2SCSUPPORT_EXPORT
bool hif_equals(const double v1, const double v2);

/// @brief Compares two reals in a safe way.
/// @param v1 The first operand.
/// @param v2 The second operand.
/// @return True if equals
HIF2SCSUPPORT_EXPORT
bool hif_equals(const float v1, const float v2);

/// @brief Compares two reals in a safe way.
/// @param v1 The first operand.
/// @param v2 The second operand.
/// @return True if equals
HIF2SCSUPPORT_EXPORT
bool hif_equals(const long double v1, const long double v2);

/// @brief This method is equivalent to mod operator of VHDL.
/// @param a is the numerator.
/// @param n is the divisor.
/// @return The resulting remainder.
HIF2SCSUPPORT_EXPORT
long long int hif_mod(const long long int a, const long long int n);

/// @brief This method maps xor reduce on integers.
/// @param v is the value.
/// @return The result.
HIF2SCSUPPORT_EXPORT
bool hif_xorrd(const unsigned long long int v);

/// @brief This method return the last value of given port/signal performing
/// the checking on given last and prev parameters. This function is used
/// to map VHDL attribute last_value.
/// @param s The port or signal.
/// @param last The last value of the port.
/// @param prev The previous value of the port.
/// @return The last value o the port.
template <typename T, typename P> T hif_lastValue(P &s, T &last, T &prev);

/// @brief This method return the last value of given variable performing
/// the checking on given last and prev parameters. This function is used
/// to map VHDL attribute last_value.
/// @note Used by a2tool.
/// @param s The variable.
/// @param last The last value of the port.
/// @param prev The previous value of the port.
/// @return The last value o the port.
template <typename T, typename P> T hif_lastValue_var(const P &s, T &last, T &prev);

/// @name Wrapper functions for sign extensions.
/// @{
template <int size2, int size1> sc_dt::sc_lv<size2> hif_sxt(sc_dt::sc_lv<size1> arg);

template <int size2, int size1> sc_dt::sc_bv<size2> hif_sxt(sc_dt::sc_bv<size1> arg);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int size2, int size1> hdtlib::hl_lv_t<size2> hif_sxt(hdtlib::hl_lv_t<size1> arg);

template <int size2, int size1> hdtlib::hl_bv_t<size2> hif_sxt(hdtlib::hl_bv_t<size1> arg);
#endif
/// @}

/// @name Relationals for logicss.
/// @{

HIF2SCSUPPORT_EXPORT
sc_dt::sc_logic hif__op_lt(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2);
HIF2SCSUPPORT_EXPORT
sc_dt::sc_logic hif__op_gt(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2);
HIF2SCSUPPORT_EXPORT
sc_dt::sc_logic hif__op_le(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2);
HIF2SCSUPPORT_EXPORT
sc_dt::sc_logic hif__op_ge(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2);

template <int W> sc_dt::sc_logic hif__op_lt_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2);
template <int W> sc_dt::sc_logic hif__op_gt_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2);
template <int W> sc_dt::sc_logic hif__op_le_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2);
template <int W> sc_dt::sc_logic hif__op_ge_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2);
template <int W> sc_dt::sc_logic hif__op_lt_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2);
template <int W> sc_dt::sc_logic hif__op_gt_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2);
template <int W> sc_dt::sc_logic hif__op_le_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2);
template <int W> sc_dt::sc_logic hif__op_ge_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
HIF2SCSUPPORT_EXPORT
hdtlib::hl_logic_t hif__op_lt(const hdtlib::hl_logic_t &v1, const hdtlib::hl_logic_t &v2);
HIF2SCSUPPORT_EXPORT
hdtlib::hl_logic_t hif__op_gt(const hdtlib::hl_logic_t &v1, const hdtlib::hl_logic_t &v2);
HIF2SCSUPPORT_EXPORT
hdtlib::hl_logic_t hif__op_le(const hdtlib::hl_logic_t &v1, const hdtlib::hl_logic_t &v2);
HIF2SCSUPPORT_EXPORT
hdtlib::hl_logic_t hif__op_ge(const hdtlib::hl_logic_t &v1, const hdtlib::hl_logic_t &v2);

template <int W> hdtlib::hl_logic_t hif__op_lt_unsigned(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2);
template <int W> hdtlib::hl_logic_t hif__op_gt_unsigned(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2);
template <int W> hdtlib::hl_logic_t hif__op_le_unsigned(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2);
template <int W> hdtlib::hl_logic_t hif__op_ge_unsigned(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2);
#endif

/// @}

/// @name Relational comparison between arrays.
/// @{
/// @brief This method allow to compare two equal-size arrays element by element
/// to check whether they are equal.
/// @param param1 The inner type of first array.
/// @param param2 The inner type of second array.
/// @return True if they are equal, False otherwise.
template <typename T>
typename std::enable_if<!std::is_pointer<T>::value, bool>::type hif_arrayEquals(T param1, T param2);

/// @name Relational comparison between arrays.
/// @{
/// @brief This method allow to compare two equal-size arrays element by element
/// to check whether they are equal.
/// @param param1 The inner type of first array.
/// @param param2 The inner type of second array.
/// @return True if they are equal, False otherwise.
template <typename T, size_t size>
typename std::enable_if<std::is_pointer<T>::value, bool>::type
hif_arrayEquals(typename std::remove_pointer<T>::type (&param1)[size], T param2);

/// @name Relational comparison between arrays.
/// @{
/// @brief This method allow to compare two equal-size arrays element by element
/// to check whether they are equal.
/// @param param1 The inner type of first array.
/// @param param2 The inner type of second array.
/// @return True if they are equal, False otherwise.
template <typename T, size_t size>
typename std::enable_if<std::is_pointer<T>::value, bool>::type
hif_arrayEquals(T param1, typename std::remove_pointer<T>::type (&param2)[size]);

/// @name Relational comparison between arrays.
/// @{
/// @brief This method allow to compare two equal-size arrays element by element
/// to check whether they are equal.
/// @param param1 The inner type of first array.
/// @param param2 The inner type of second array.
/// @return True if they are equal, False otherwise.
template <typename T> bool hif_arrayEquals(T *const &param1, T *const &param2);

/// @brief This method allow to compare two bi-dimensional equal-size arrays
/// element by element to check whether they are equal.
/// @param param1 The first array.
/// @param param2 The second array.
/// @return True if they are equal, False otherwise.
template <typename T, size_t size> bool hif_arrayEquals(T (&param1)[size], T (&param2)[size]);

/// @brief This method allow to compare two bi-dimensional equal-size arrays
/// element by element to check whether they are equal.
/// @param param1 The first array.
/// @param param2 The second array.
/// @return True if they are equal, False otherwise.
template <typename P, typename T, size_t size> bool hif_arrayEquals(sc_core::sc_vector<P> &param1, T (&param2)[size]);

/// @brief This method allow to compare two bi-dimensional equal-size arrays
/// element by element to check whether they are equal.
/// @param param1 The first array.
/// @param param2 The second array.
/// @return True if they are equal, False otherwise.
template <typename P, typename T, size_t size> bool hif_arrayEquals(T (&param1)[size], sc_core::sc_vector<P> &param2);

///@}

/// @name Logic comparisons.
/// @{

/// This method allows to compare two logic types, considering 'X' as false.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param sign Whether extend with sign.
/// @return The resulting logic value.
template <int W1, int W2>
sc_dt::sc_logic hif_logicEquals(sc_dt::sc_lv<W1> param1, sc_dt::sc_lv<W2> param2, const bool sign);

/// This method allows to compare two logic types, considering 'X' as false.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @return The resulting logic value.
HIF2SCSUPPORT_EXPORT
sc_dt::sc_logic hif_logicEquals(const sc_dt::sc_logic &param1, const sc_dt::sc_logic &param2);

/// This method allows to compare two arrays of logic types, considering 'X' as false.
/// @param param1 The first array.
/// @param param2 The second array.
/// @param sign True if signed comparison must be performed.
/// @return The resulting logic value.
template <typename T1, typename T2, size_t size>
sc_dt::sc_logic hif_logicEquals(T1 (&param1)[size], T2 (&param2)[size], const bool sign);

/// This method allows to compare two logic types, considering 'X' and 'Z' as dontcares.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param param3 If true consider also 'X' as dontcare.
/// @param sign True if signed comparison must be performed.
/// @return The resulting value.
template <int W1, int W2>
bool hif_caseXZ(sc_dt::sc_lv<W1> param1, sc_dt::sc_lv<W2> param2, const bool param3, const bool sign);

/// This method allows to compare two logic types, considering 'X' and 'Z' as dontcare.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param param3 If true consider also 'X' as dontcare.
/// @return The resulting logic value.
HIF2SCSUPPORT_EXPORT
bool hif_caseXZ(const sc_dt::sc_logic &param1, const sc_dt::sc_logic &param2, const bool param3);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

/// This method allows to compare two logic types, considering 'X' as false.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param sign Whether extend with sign.
/// @return The resulting logic value.
template <int W1, int W2>
hdtlib::hl_logic_t hif_logicEquals_hdtlib(hdtlib::hl_lv_t<W1> param1, hdtlib::hl_lv_t<W2> param2, const bool sign);

/// This method allows to compare two logic types, considering 'X' as false.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @return The resulting logic value.
HIF2SCSUPPORT_EXPORT
hdtlib::hl_logic_t hif_logicEquals_hdtlib(hdtlib::hl_logic_t param1, hdtlib::hl_logic_t param2);

/// This method allows to compare two arrays of logic types, considering 'X' as false.
/// @param param1 The first array.
/// @param param2 The second array.
/// @param sign True if signed comparison must be performed.
/// @return The resulting logic value.
template <typename T1, typename T2, size_t size>
hdtlib::hl_logic_t hif_logicEquals_hdtlib(T1 (&param1)[size], T2 (&param2)[size], const bool sign);

/// This method allows to compare two logic types, considering 'X' and 'Z' as dontcares.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param param3 If true consider also 'X' as dontcare.
/// @param sign True if signed comparison must be performed.
/// @return The resulting value.
template <int W1, int W2>
bool hif_caseXZ(hdtlib::hl_lv_t<W1> param1, hdtlib::hl_lv_t<W2> param2, const bool param3, const bool sign);

/// This method allows to compare two logic types, considering 'X' and 'Z' as dontcare.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param param3 If true consider also 'X' as dontcare.
/// @return The resulting logic value.
HIF2SCSUPPORT_EXPORT
bool hif_caseXZ(hdtlib::hl_logic_t param1, hdtlib::hl_logic_t param2, const bool param3);

#endif

///@}

/// @name Shift operations
/// @{
/// @brief This method performs the shift between logic vectors

template <int size1, int size2>
sc_dt::sc_lv<size1> hif_op_shift_left(sc_dt::sc_lv<size1> param1, sc_dt::sc_lv<size2> param2);

template <int size1, int size2>
sc_dt::sc_lv<size1> hif_op_shift_right_arith(sc_dt::sc_lv<size1> param1, sc_dt::sc_lv<size2> param2);

template <int size1, int size2>
sc_dt::sc_lv<size1> hif_op_shift_right_logic(sc_dt::sc_lv<size1> param1, sc_dt::sc_lv<size2> param2);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_op_shift_left(hdtlib::hl_lv_t<size1> param1, hdtlib::hl_lv_t<size2> param2);

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_op_shift_right_arith(hdtlib::hl_lv_t<size1> param1, hdtlib::hl_lv_t<size2> param2);

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_op_shift_right_logic(hdtlib::hl_lv_t<size1> param1, hdtlib::hl_lv_t<size2> param2);

#endif

/// @}

/// @name Reverse operations
/// @{

HIF2SCSUPPORT_EXPORT
std::string hif_reverse(const std::string &p);

template <int W> sc_dt::sc_bv<W> hif_reverse(const sc_dt::sc_bv<W> &p);

template <int W> sc_dt::sc_lv<W> hif_reverse(const sc_dt::sc_lv<W> &p);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int W> hdtlib::hl_bv_t<W> hif_reverse(const hdtlib::hl_bv_t<W> &p);

template <int W> hdtlib::hl_lv_t<W> hif_reverse(const hdtlib::hl_lv_t<W> &p);

#endif

/// @}

} // namespace hif_systemc_extensions

#include "hif2scSupport/hif_systemc_extension_ArrayConcat.hpp"
#include "hif2scSupport/hif_systemc_extensions_HifAggregate.hpp"

#include "hif2scSupport/hif_systemc_extensions.i.hpp"

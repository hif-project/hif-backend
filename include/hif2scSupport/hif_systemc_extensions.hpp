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
auto hif_getResourcePath() -> std::string;
/// @brief Gets the resource file name.
HIF2SCSUPPORT_EXPORT
auto hif_getResourceFileName(const std::string &name) -> std::string;

/// @brief Compares two reals in a safe way.
/// @param v1 The first operand.
/// @param v2 The second operand.
/// @return True if equals
HIF2SCSUPPORT_EXPORT
auto hif_equals(double v1, double v2) -> bool;

/// @brief Compares two reals in a safe way.
/// @param v1 The first operand.
/// @param v2 The second operand.
/// @return True if equals
HIF2SCSUPPORT_EXPORT
auto hif_equals(float v1, float v2) -> bool;

/// @brief Compares two reals in a safe way.
/// @param v1 The first operand.
/// @param v2 The second operand.
/// @return True if equals
HIF2SCSUPPORT_EXPORT
auto hif_equals(long double v1, long double v2) -> bool;

/// @brief This method is equivalent to mod operator of VHDL.
/// @param a is the numerator.
/// @param n is the divisor.
/// @return The resulting remainder.
HIF2SCSUPPORT_EXPORT
auto hif_mod(long long int a, long long int n) -> long long int;

/// @brief This method maps xor reduce on integers.
/// @param v is the value.
/// @return The result.
HIF2SCSUPPORT_EXPORT
auto hif_xorrd(unsigned long long int v) -> bool;

/// @brief This method return the last value of given port/signal performing
/// the checking on given last and prev parameters. This function is used
/// to map VHDL attribute last_value.
/// @param s The port or signal.
/// @param last The last value of the port.
/// @param prev The previous value of the port.
/// @return The last value o the port.
template <typename T, typename P> auto hif_lastValue(P &s, T &last, T &prev) -> T;

/// @brief This method return the last value of given variable performing
/// the checking on given last and prev parameters. This function is used
/// to map VHDL attribute last_value.
/// @note Used by a2tool.
/// @param s The variable.
/// @param last The last value of the port.
/// @param prev The previous value of the port.
/// @return The last value o the port.
template <typename T, typename P> auto hif_lastValue_var(const P &s, T &last, T &prev) -> T;

/// @name Wrapper functions for sign extensions.
/// @{
template <int size2, int size1> auto hif_sxt(sc_dt::sc_lv<size1> arg) -> sc_dt::sc_lv<size2>;

template <int size2, int size1> auto hif_sxt(sc_dt::sc_bv<size1> arg) -> sc_dt::sc_bv<size2>;

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int size2, int size1> hdtlib::hl_lv_t<size2> hif_sxt(hdtlib::hl_lv_t<size1> arg);

template <int size2, int size1> hdtlib::hl_bv_t<size2> hif_sxt(hdtlib::hl_bv_t<size1> arg);
#endif
/// @}

/// @name Relationals for logicss.
/// @{

HIF2SCSUPPORT_EXPORT
auto hif_op_lt(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2) -> sc_dt::sc_logic;
HIF2SCSUPPORT_EXPORT
auto hif_op_gt(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2) -> sc_dt::sc_logic;
HIF2SCSUPPORT_EXPORT
auto hif_op_le(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2) -> sc_dt::sc_logic;
HIF2SCSUPPORT_EXPORT
auto hif_op_ge(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2) -> sc_dt::sc_logic;

template <int W> auto hif_op_lt_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic;
template <int W> auto hif_op_gt_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic;
template <int W> auto hif_op_le_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic;
template <int W> auto hif_op_ge_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic;
template <int W> auto hif_op_lt_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic;
template <int W> auto hif_op_gt_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic;
template <int W> auto hif_op_le_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic;
template <int W> auto hif_op_ge_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic;

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
auto hif_arrayEquals(T param1, T param2) -> typename std::enable_if<!std::is_pointer<T>::value, bool>::type;

/// @name Relational comparison between arrays.
/// @{
/// @brief This method allow to compare two equal-size arrays element by element
/// to check whether they are equal.
/// @param param1 The inner type of first array.
/// @param param2 The inner type of second array.
/// @return True if they are equal, False otherwise.
template <typename T, size_t size>
auto
hif_arrayEquals(typename std::remove_pointer<T>::type (&param1)[size], T param2) -> typename std::enable_if<std::is_pointer<T>::value, bool>::type;

/// @name Relational comparison between arrays.
/// @{
/// @brief This method allow to compare two equal-size arrays element by element
/// to check whether they are equal.
/// @param param1 The inner type of first array.
/// @param param2 The inner type of second array.
/// @return True if they are equal, False otherwise.
template <typename T, size_t size>
auto
hif_arrayEquals(T param1, typename std::remove_pointer<T>::type (&param2)[size]) -> typename std::enable_if<std::is_pointer<T>::value, bool>::type;

/// @name Relational comparison between arrays.
/// @{
/// @brief This method allow to compare two equal-size arrays element by element
/// to check whether they are equal.
/// @param param1 The inner type of first array.
/// @param param2 The inner type of second array.
/// @return True if they are equal, False otherwise.
template <typename T> auto hif_arrayEquals(T *const &param1, T *const &param2) -> bool;

/// @brief This method allow to compare two bi-dimensional equal-size arrays
/// element by element to check whether they are equal.
/// @param param1 The first array.
/// @param param2 The second array.
/// @return True if they are equal, False otherwise.
template <typename T, size_t size> auto hif_arrayEquals(T (&param1)[size], T (&param2)[size]) -> bool;

/// @brief This method allow to compare two bi-dimensional equal-size arrays
/// element by element to check whether they are equal.
/// @param param1 The first array.
/// @param param2 The second array.
/// @return True if they are equal, False otherwise.
template <typename P, typename T, size_t size> auto hif_arrayEquals(sc_core::sc_vector<P> &param1, T (&param2)[size]) -> bool;

/// @brief This method allow to compare two bi-dimensional equal-size arrays
/// element by element to check whether they are equal.
/// @param param1 The first array.
/// @param param2 The second array.
/// @return True if they are equal, False otherwise.
template <typename P, typename T, size_t size> auto hif_arrayEquals(T (&param1)[size], sc_core::sc_vector<P> &param2) -> bool;

///@}

/// @name Logic comparisons.
/// @{

/// This method allows to compare two logic types, considering 'X' as false.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param sign Whether extend with sign.
/// @return The resulting logic value.
template <int W1, int W2>
auto hif_logicEquals(sc_dt::sc_lv<W1> param1, sc_dt::sc_lv<W2> param2, bool sign) -> sc_dt::sc_logic;

/// This method allows to compare two logic types, considering 'X' as false.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @return The resulting logic value.
HIF2SCSUPPORT_EXPORT
auto hif_logicEquals(const sc_dt::sc_logic &param1, const sc_dt::sc_logic &param2) -> sc_dt::sc_logic;

/// This method allows to compare two arrays of logic types, considering 'X' as false.
/// @param param1 The first array.
/// @param param2 The second array.
/// @param sign True if signed comparison must be performed.
/// @return The resulting logic value.
template <typename T1, typename T2, size_t size>
auto hif_logicEquals(T1 (&param1)[size], T2 (&param2)[size], bool sign) -> sc_dt::sc_logic;

/// This method allows to compare two logic types, considering 'X' and 'Z' as dontcares.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param param3 If true consider also 'X' as dontcare.
/// @param sign True if signed comparison must be performed.
/// @return The resulting value.
template <int W1, int W2>
auto hif_caseXZ(sc_dt::sc_lv<W1> param1, sc_dt::sc_lv<W2> param2, bool param3, bool sign) -> bool;

/// This method allows to compare two logic types, considering 'X' and 'Z' as dontcare.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param param3 If true consider also 'X' as dontcare.
/// @return The resulting logic value.
HIF2SCSUPPORT_EXPORT
auto hif_caseXZ(const sc_dt::sc_logic &param1, const sc_dt::sc_logic &param2, bool param3) -> bool;

#ifdef HIF2SCSUPPORT_USE_HDTLIB

/// This method allows to compare two logic types, considering 'X' as false.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param sign Whether extend with sign.
/// @return The resulting logic value.
template <int W1, int W2>
hdtlib::hl_logic_t hif_logicEquals_hdtlib(hdtlib::hl_lv_t<W1> param1, hdtlib::hl_lv_t<W2> param2, bool sign);

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
hdtlib::hl_logic_t hif_logicEquals_hdtlib(T1 (&param1)[size], T2 (&param2)[size], bool sign);

/// This method allows to compare two logic types, considering 'X' and 'Z' as dontcares.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param param3 If true consider also 'X' as dontcare.
/// @param sign True if signed comparison must be performed.
/// @return The resulting value.
template <int W1, int W2>
bool hif_caseXZ(hdtlib::hl_lv_t<W1> param1, hdtlib::hl_lv_t<W2> param2, bool param3, bool sign);

/// This method allows to compare two logic types, considering 'X' and 'Z' as dontcare.
/// @param param1 The first logic value.
/// @param param2 The second logic value.
/// @param param3 If true consider also 'X' as dontcare.
/// @return The resulting logic value.
HIF2SCSUPPORT_EXPORT
bool hif_caseXZ(hdtlib::hl_logic_t param1, hdtlib::hl_logic_t param2, bool param3);

#endif

///@}

/// @name Shift operations
/// @{
/// @brief This method performs the shift between logic vectors

template <int size1, int size2>
auto hif_op_shift_left(sc_dt::sc_lv<size1> param1, sc_dt::sc_lv<size2> param2) -> sc_dt::sc_lv<size1>;

template <int size1, int size2>
auto hif_op_shift_right_arith(sc_dt::sc_lv<size1> param1, sc_dt::sc_lv<size2> param2) -> sc_dt::sc_lv<size1>;

template <int size1, int size2>
auto hif_op_shift_right_logic(sc_dt::sc_lv<size1> param1, sc_dt::sc_lv<size2> param2) -> sc_dt::sc_lv<size1>;

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
auto hif_reverse(const std::string &p) -> std::string;

template <int W> auto hif_reverse(const sc_dt::sc_bv<W> &p) -> sc_dt::sc_bv<W>;

template <int W> auto hif_reverse(const sc_dt::sc_lv<W> &p) -> sc_dt::sc_lv<W>;

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int W> hdtlib::hl_bv_t<W> hif_reverse(const hdtlib::hl_bv_t<W> &p);

template <int W> hdtlib::hl_lv_t<W> hif_reverse(const hdtlib::hl_lv_t<W> &p);

#endif

/// @}

} // namespace hif_systemc_extensions

#include "hif2scSupport/hif_systemc_extension_ArrayConcat.hpp"
#include "hif2scSupport/hif_systemc_extensions_HifAggregate.hpp"

#include "hif2scSupport/hif_systemc_extensions.i.hpp"

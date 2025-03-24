/// @file hif_systemc_extensions.i.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "../hif_vhdl_ieee_std_logic_arith.hpp"
#include "config.hpp"

namespace hif_systemc_extensions
{

template <typename T, typename P> auto hif_lastValue(P &s, T &last, T &prev) -> T
{
    if (s.read() != prev) {
        T m  = s.read() ^ prev;
        last = (prev & m) | (last & ~m);
        prev = s.read();
    }

    return last;
}

template <typename T, typename P> auto hif_lastValue_var(const P &s, T &last, T &prev) -> T
{
    if (s != prev) {
        T m  = s ^ prev;
        last = (prev & m) | (last & ~m);
        prev = s;
    }

    return last;
}

template <int size2, int size1> auto hif_sxt(sc_dt::sc_lv<size1> arg) -> sc_dt::sc_lv<size2>
{
    if (size2 <= 0) {
        return sc_dt::sc_lv<size2>();
    }
    if (size2 <= arg.length()) {
        return arg.range(size2 - 1, 0);
    }

    std::string xStr = arg.to_string();
    char c           = arg[arg.length() - 1].to_char();
    for (int i = arg.length(); i < size2; ++i) {
        xStr = c + xStr;
    }
    return sc_dt::sc_lv<size2>(xStr.c_str());
}

template <int size2, int size1> auto hif_sxt(sc_dt::sc_bv<size1> arg) -> sc_dt::sc_bv<size2>
{
    return sc_dt::sc_bv<size2>(hif_sxt<size2>(sc_dt::sc_lv<size1>(arg)));
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int size2, int size1> hdtlib::hl_lv_t<size2> hif_sxt(hdtlib::hl_lv_t<size1> arg)
{
    if (size2 <= 0)
        return hdtlib::hl_lv_t<size2>();
    if (size2 <= size1)
        return arg.template range<size2>(size2 - 1, 0);

    std::string xStr = arg.to_string();
    char c           = arg[size1 - 1].to_char();
    for (int i = size1; i < size2; ++i) {
        xStr = c + xStr;
    }
    return hdtlib::hl_lv_t<size2>(xStr.c_str());
}

template <int size2, int size1> hdtlib::hl_bv_t<size2> hif_sxt(hdtlib::hl_bv_t<size1> arg)
{
    return hdtlib::hl_bv_t<size2>(hif_sxt<size2>(hdtlib::hl_lv_t<size1>(arg)));
}
#endif

// ///////////////////////////////////////////////////////////////////
// Relationals for logics
// ///////////////////////////////////////////////////////////////////

template <int W> auto hif_op_lt_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic
{
    if (!v1.is_01() || !v2.is_01()) {
        return sc_dt::sc_logic('X');
    }
    return sc_dt::sc_logic(sc_dt::sc_bigint<W>(v1) < sc_dt::sc_bigint<W>(v2));
}

template <int W> auto hif_op_gt_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic
{
    if (!v1.is_01() || !v2.is_01()) {
        return sc_dt::sc_logic('X');
    }
    return sc_dt::sc_logic(sc_dt::sc_bigint<W>(v1) > sc_dt::sc_bigint<W>(v2));
}

template <int W> auto hif_op_le_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic
{
    if (!v1.is_01() || !v2.is_01()) {
        return sc_dt::sc_logic('X');
    }
    return sc_dt::sc_logic(sc_dt::sc_bigint<W>(v1) <= sc_dt::sc_bigint<W>(v2));
}

template <int W> auto hif_op_ge_signed(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic
{
    if (!v1.is_01() || !v2.is_01()) {
        return sc_dt::sc_logic('X');
    }
    return sc_dt::sc_logic(sc_dt::sc_bigint<W>(v1) >= sc_dt::sc_bigint<W>(v2));
}

template <int W> auto hif_op_lt_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic
{
    if (!v1.is_01() || !v2.is_01()) {
        return sc_dt::sc_logic('X');
    }
    return sc_dt::sc_logic(sc_dt::sc_biguint<W>(v1) < sc_dt::sc_biguint<W>(v2));
}

template <int W> auto hif_op_gt_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic
{
    if (!v1.is_01() || !v2.is_01()) {
        return sc_dt::sc_logic('X');
    }
    return sc_dt::sc_logic(sc_dt::sc_biguint<W>(v1) > sc_dt::sc_biguint<W>(v2));
}

template <int W> auto hif_op_le_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic
{
    if (!v1.is_01() || !v2.is_01()) {
        return sc_dt::sc_logic('X');
    }
    return sc_dt::sc_logic(sc_dt::sc_biguint<W>(v1) <= sc_dt::sc_biguint<W>(v2));
}

template <int W> auto hif_op_ge_unsigned(const sc_dt::sc_lv<W> &v1, const sc_dt::sc_lv<W> &v2) -> sc_dt::sc_logic
{
    if (!v1.is_01() || !v2.is_01()) {
        return sc_dt::sc_logic('X');
    }
    return sc_dt::sc_logic(sc_dt::sc_biguint<W>(v1) >= sc_dt::sc_biguint<W>(v2));
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int W> hdtlib::hl_logic_t hif__op_lt_signed(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return hdtlib::hl_logic_t(hdtlib::hl_int_t<W>(v1) < hdtlib::hl_int_t<W>(v2));
}

template <int W> hdtlib::hl_logic_t hif__op_gt_signed(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return hdtlib::hl_logic_t(hdtlib::hl_int_t<W>(v1) > hdtlib::hl_int_t<W>(v2));
}

template <int W> hdtlib::hl_logic_t hif__op_le_signed(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return hdtlib::hl_logic_t(hdtlib::hl_int_t<W>(v1) <= hdtlib::hl_int_t<W>(v2));
}

template <int W> hdtlib::hl_logic_t hif__op_ge_signed(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return hdtlib::hl_logic_t(hdtlib::hl_int_t<W>(v1) >= hdtlib::hl_int_t<W>(v2));
}

template <int W> hdtlib::hl_logic_t hif__op_lt_unsigned(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return hdtlib::hl_logic_t(hdtlib::hl_uint_t<W>(v1) < hdtlib::hl_uint_t<W>(v2));
}

template <int W> hdtlib::hl_logic_t hif__op_gt_unsigned(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return hdtlib::hl_logic_t(hdtlib::hl_uint_t<W>(v1) > hdtlib::hl_uint_t<W>(v2));
}

template <int W> hdtlib::hl_logic_t hif__op_le_unsigned(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return hdtlib::hl_logic_t(hdtlib::hl_uint_t<W>(v1) <= hdtlib::hl_uint_t<W>(v2));
}

template <int W> hdtlib::hl_logic_t hif__op_ge_unsigned(const hdtlib::hl_lv_t<W> &v1, const hdtlib::hl_lv_t<W> &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return hdtlib::hl_logic_t(hdtlib::hl_uint_t<W>(v1) >= hdtlib::hl_uint_t<W>(v2));
}

#endif

// ///////////////////////////////////////////////////////////////////
// Equals for arrays
// ///////////////////////////////////////////////////////////////////
template <typename T>
auto hif_arrayEquals(T param1, T param2) -> typename std::enable_if<!std::is_pointer<T>::value, bool>::type
{
    return param1 == param2;
}

template <typename T, size_t size>
auto hif_arrayEquals(typename std::remove_pointer<T>::type (&param1)[size], T param2) ->
    typename std::enable_if<std::is_pointer<T>::value, bool>::type
{
    for (size_t i = 0; i < size; ++i) {
        if (!hif_arrayEquals(param1[i], param2[i])) {
            return false;
        }
    }
    return true;
}

template <typename T, size_t size>
auto hif_arrayEquals(T param1, typename std::remove_pointer<T>::type (&param2)[size]) ->
    typename std::enable_if<std::is_pointer<T>::value, bool>::type
{
    for (size_t i = 0; i < size; ++i) {
        if (!hif_arrayEquals(param1[i], param2[i])) {
            return false;
        }
    }
    return true;
}

template <typename T> auto hif_arrayEquals(T *const & /*param1*/, T *const & /*param2*/) -> bool
{
    assert(false);
    return false;
}

template <typename T, size_t size> auto hif_arrayEquals(T (&param1)[size], T (&param2)[size]) -> bool
{
    for (size_t i = 0; i < size; ++i) {
        if (!hif_arrayEquals(param1[i], param2[i])) {
            return false;
        }
    }
    return true;
}

template <typename P, typename T, size_t size>
auto hif_arrayEquals(sc_core::sc_vector<P> &param1, T (&param2)[size]) -> bool
{
    for (size_t i = 0; i < size; ++i) {
        if (!hif_arrayEquals(param1[i].read(), param2[i])) {
            return false;
        }
    }
    return true;
}

template <typename P, typename T, size_t size>
auto hif_arrayEquals(T (&param1)[size], sc_core::sc_vector<P> &param2) -> bool
{
    for (size_t i = 0; i < size; ++i) {
        if (!hif_arrayEquals(param1[i], param2[i].read())) {
            return false;
        }
    }
    return true;
}

template <int W1, int W2>
auto hif_logicEquals(sc_dt::sc_lv<W1> param1, sc_dt::sc_lv<W2> param2, bool sign) -> sc_dt::sc_logic
{
    enum Const { SIZE = (W1 > W2) ? W1 : W2 };

    sc_dt::sc_lv<SIZE> l1;
    sc_dt::sc_lv<SIZE> l2;
    if (sign) {
        if (W1 != SIZE) {
            l1 = hif_sxt<SIZE>(param1);
        } else {
            l1 = param1;
        }

        if (W2 != SIZE) {
            l2 = hif_sxt<SIZE>(param2);
        } else {
            l2 = param2;
        }
    } else {
        // extend with zeros
        l1 = param1;
        l2 = param2;
    }

    if (param1.is_01() && param2.is_01()) {
        return sc_dt::sc_logic(l1 == l2);
    }

    return sc_dt::sc_logic(~(sc_dt::sc_logic((l1 ^ l2).or_reduce())));
}

template <typename T1, typename T2, size_t size>
auto hif_logicEquals(T1 (&param1)[size], T2 (&param2)[size], bool sign) -> sc_dt::sc_logic
{
    sc_dt::sc_logic res = hif_logicEquals(param1[0], param2[0], sign);
    for (size_t i = 1; i < size; ++i) {
        res &= hif_logicEquals(param1[i], param2[i], sign);
    }

    return res;
}

template <int W1, int W2>
auto hif_caseXZ(sc_dt::sc_lv<W1> param1, sc_dt::sc_lv<W2> param2, bool param3, bool sign) -> bool
{
    enum Const { SIZE = (W1 > W2) ? W1 : W2 };

    sc_dt::sc_lv<SIZE> l1;
    sc_dt::sc_lv<SIZE> l2;
    if (sign) {
        if (W1 != SIZE) {
            l1 = hif_sxt<SIZE>(param1);
        } else {
            l1 = param1;
        }

        if (W2 != SIZE) {
            l2 = hif_sxt<SIZE>(param2);
        } else {
            l2 = param2;
        }
    } else {
        // extend with zeros
        l1 = param1;
        l2 = param2;
    }

    const std::string s1 = l1.to_string();
    const std::string s2 = l2.to_string();
    for (std::string::size_type i = 0; i < s1.size(); ++i) {
        if (s1[i] == s2[i]) {
            continue;
        }
        if (s1[i] == 'Z' || s2[i] == 'Z') {
            continue;
        } if (param3 && (s1[i] == 'X' || s2[i] == 'X'))
            continue;

        return false;
    }

    return true;
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int W1, int W2>
hdtlib::hl_logic_t hif_logicEquals_hdtlib(hdtlib::hl_lv_t<W1> param1, hdtlib::hl_lv_t<W2> param2, bool sign)
{
    enum Const { SIZE = (W1 > W2) ? W1 : W2 };

    hdtlib::hl_lv_t<SIZE> l1;
    hdtlib::hl_lv_t<SIZE> l2;
    if (sign) {
        if (W1 != SIZE)
            l1 = hif_sxt<SIZE>(param1);
        else
            l1 = param1;

        if (W2 != SIZE)
            l2 = hif_sxt<SIZE>(param2);
        else
            l2 = param2;
    } else {
        // extend with zeros
        l1 = param1;
        l2 = param2;
    }

    if (param1.is_01() && param2.is_01())
        return hdtlib::hl_logic_t(l1 == l2);

    return hdtlib::hl_logic_t(~(hdtlib::hl_logic_t((l1 ^ l2).or_reduce())));
}

template <typename T1, typename T2, size_t size>
hdtlib::hl_logic_t hif_logicEquals_hdtlib(T1 (&param1)[size], T2 (&param2)[size], bool sign)
{
    hdtlib::hl_logic_t res = hif_logicEquals_hdtlib(param1[0], param2[0], sign);
    for (size_t i = 1; i < size; ++i) {
        res &= hif_logicEquals_hdtlib(param1[i], param2[i], sign);
    }

    return res;
}

template <int W1, int W2>
bool hif_caseXZ(hdtlib::hl_lv_t<W1> param1, hdtlib::hl_lv_t<W2> param2, bool param3, bool sign)
{
    enum Const { SIZE = (W1 > W2) ? W1 : W2 };

    hdtlib::hl_lv_t<SIZE> l1;
    hdtlib::hl_lv_t<SIZE> l2;
    if (sign) {
        if (W1 != SIZE)
            l1 = hif_sxt<SIZE>(param1);
        else
            l1 = param1;

        if (W2 != SIZE)
            l2 = hif_sxt<SIZE>(param2);
        else
            l2 = param2;
    } else {
        // extend with zeros
        l1 = param1;
        l2 = param2;
    }

    const std::string s1 = l1.to_string();
    const std::string s2 = l2.to_string();
    for (std::string::size_type i = 0; i < s1.size(); ++i) {
        if (s1[i] == s2[i])
            continue;
        else if (s1[i] == 'Z' || s2[i] == 'Z')
            continue;
        else if (param3 && (s1[i] == 'X' || s2[i] == 'X'))
            continue;

        return false;
    }

    return true;
}

#endif

template <int size1, int size2>
auto hif_op_shift_left(sc_dt::sc_lv<size1> param1, sc_dt::sc_lv<size2> param2) -> sc_dt::sc_lv<size1>
{
    if (!param2.is_01()) {
        return sc_dt::sc_lv<size1>();
    }

    param1 = param1 << param2.to_uint64();
    return param1;
}

template <int size1, int size2>
auto hif_op_shift_right_arith(sc_dt::sc_lv<size1> param1, sc_dt::sc_lv<size2> param2) -> sc_dt::sc_lv<size1>
{
    if (!param2.is_01()) {
        return sc_dt::sc_lv<size1>();
    }

    std::string s   = param1.to_string();
    const char sign = s[0];

    param1 = param1 >> param2.to_uint64();

    if (sign == '0') {
        return param1;
    }

    for (unsigned long long int i = 0; i < param2.to_uint64(); ++i) {
        param1[i] = sign;
    }

    return param1;
}

template <int size1, int size2>
auto hif_op_shift_right_logic(sc_dt::sc_lv<size1> param1, sc_dt::sc_lv<size2> param2) -> sc_dt::sc_lv<size1>
{
    if (!param2.is_01()) {
        return sc_dt::sc_lv<size1>();
    }

    param1 = param1 >> param2.to_uint64();
    return param1;
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_op_shift_left(hdtlib::hl_lv_t<size1> param1, hdtlib::hl_lv_t<size2> param2)
{
    if (!param2.is_01())
        return hdtlib::hl_lv_t<size1>();

    param1 = param1 << param2.to_uint64();
    return param1;
}

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_op_shift_right_arith(hdtlib::hl_lv_t<size1> param1, hdtlib::hl_lv_t<size2> param2)
{
    if (!param2.is_01())
        return hdtlib::hl_lv_t<size1>();

    std::string s   = param1.to_string();
    const char sign = s[0];

    param1 = param1 >> param2.to_uint64();

    if (sign == '0')
        return param1;

    for (unsigned long long int i = 0; i < param2.to_uint64(); ++i) {
        param1[i] = sign;
    }

    return param1;
}

template <int size1, int size2>
hdtlib::hl_lv_t<size1> hif_op_shift_right_logic(hdtlib::hl_lv_t<size1> param1, hdtlib::hl_lv_t<size2> param2)
{
    if (!param2.is_01())
        return hdtlib::hl_lv_t<size1>();

    param1 = param1 >> param2.to_uint64();
    return param1;
}

#endif

// Reverse operations

template <int W> auto hif_reverse(const sc_dt::sc_bv<W> &p) -> sc_dt::sc_bv<W>
{
    sc_dt::sc_bv<W> ret = p;
    return ret.reverse();
}

template <int W> auto hif_reverse(const sc_dt::sc_lv<W> &p) -> sc_dt::sc_lv<W>
{
    sc_dt::sc_lv<W> ret = p;
    return ret.reverse();
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

template <int W> hdtlib::hl_bv_t<W> hif_reverse(const hdtlib::hl_bv_t<W> &p)
{
    std::string s = p.to_string();
    std::reverse(s.begin(), s.end());
    hdtlib::hl_bv_t<W> ret = "00" + s;
    return ret;
}

template <int W> hdtlib::hl_lv_t<W> hif_reverse(const hdtlib::hl_lv_t<W> &p)
{
    std::string s = p.to_string();
    std::reverse(s.begin(), s.end());
    hdtlib::hl_lv_t<W> ret = "00" + s;
    return ret;
}

#endif

} // namespace hif_systemc_extensions

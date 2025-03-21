/// @file hif_vhdl_ieee_std_logic_textio.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2scSupport/hif_vhdl_ieee_std_logic_textio.hpp"
#include "hif2scSupport/hif_vhdl_standard.hpp"

#if (defined _MSC_VER)
#pragma warning(disable : 4127)
#endif

namespace hif_vhdl_ieee_std_logic_textio
{

namespace /* anon */
{

auto trim(const std::string &str, const std::string &whitespace = " \t") -> std::string
{
    const std::size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) {
        return ""; // no content
    }

    const std::size_t strEnd   = str.find_last_not_of(whitespace);
    const std::size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

void _warn(const std::string &s)
{
    hif_vhdl_standard::hif_vhdl_assert(
        false, "Found SystemC unsupported logic value: " + s, hif_vhdl_standard::hif_vhdl_warning);
}

auto _isValidLogic(const std::string &s) -> bool
{
    if (s.size() != 1) {
        return false;
    }
    if (s == "1" || s == "0" || s == "Z" || s == "z" || s == "X" || s == "x") {
        return true;
    }
    if (s == "W" || s != "w" || s == "L" || s == "l" || s == "H" || s == "h" || s == "U" || s == "u" || s == "-") {
        _warn(s);
        return true;
    }

    return false;
}

auto _toLogic(const std::string &s) -> char
{
    if (s == "1") {
        return '1';
    }
    if (s == "0")
        return '0';
    else if (s == "x" || s == "X")
        return 'X';
    else if (s == "z" || s == "Z")
        return 'Z';
    else if (s == "u" || s == "U")
        return 'X';
    else if (s == "l" || s == "L")
        return '0';
    else if (s == "h" || s == "H")
        return '1';
    else if (s == "w" || s == "W")
        return 'X';
    // else if (s == "-" )
    return 'X';
}

auto _isValidLogicVector(const std::string &s) -> bool
{
    const std::string::size_type l = s.size();
    if (l < 1) {
        return false;
    }
    for (std::string::size_type i = 0; i < l; ++i) {
        std::string tmp;
        tmp = s[i];
        if (!_isValidLogic(tmp)) {
            return false;
        }
    }

    return true;
}

auto _toLogicVector(const std::string &s) -> std::string
{
    const std::string::size_type l = s.size();
    std::string ret(s);
    for (std::string::size_type i = 0; i < l; ++i) {
        std::string tmp;
        tmp    = s[i];
        ret[i] = _toLogic(tmp);
    }

    return ret;
}

auto char2QuadBits(const char c, bool &good) -> std::string
{
    std::string ret;
    good = true;
    switch (c) {
    case '0':
        ret = "0000";
        break;
    case '1':
        ret = "0001";
        break;
    case '2':
        ret = "0010";
        break;
    case '3':
        ret = "0011";
        break;
    case '4':
        ret = "0100";
        break;
    case '5':
        ret = "0101";
        break;
    case '6':
        ret = "0110";
        break;
    case '7':
        ret = "0111";
        break;
    case '8':
        ret = "1000";
        break;
    case '9':
        ret = "1001";
        break;
    case 'A':
    case 'a':
        ret = "1010";
        break;
    case 'B':
    case 'b':
        ret = "1011";
        break;
    case 'C':
    case 'c':
        ret = "1100";
        break;
    case 'D':
    case 'd':
        ret = "1101";
        break;
    case 'E':
    case 'e':
        ret = "1110";
        break;
    case 'F':
    case 'f':
        ret = "1111";
        break;
    default:
        hif_vhdl_standard::hif_vhdl_assert(
            false, std::string("Found unsupported hex value: ") + c, hif_vhdl_standard::hif_vhdl_error);
        good = false;
    }

    return ret;
}

auto string2QuadBits(const std::string &s, bool &good) -> std::string
{
    std::string ret;
    for (char i : s) {
        ret += char2QuadBits(i, good);
        if (!good) {
            return ret;
        }
    }

    return ret;
}

auto char2TriBits(const char c, bool &good) -> std::string
{
    std::string ret;
    good = true;
    switch (c) {
    case '0':
        ret = "000";
        break;
    case '1':
        ret = "001";
        break;
    case '2':
        ret = "010";
        break;
    case '3':
        ret = "011";
        break;
    case '4':
        ret = "100";
        break;
    case '5':
        ret = "101";
        break;
    case '6':
        ret = "110";
        break;
    case '7':
        ret = "111";
        break;
    default:
        hif_vhdl_standard::hif_vhdl_assert(
            false, std::string("Found unsupported octal value: ") + c, hif_vhdl_standard::hif_vhdl_error);
        good = false;
    }

    return ret;
}

auto string2TriBits(const std::string &s, bool &good) -> std::string
{
    std::string ret;
    for (char i : s) {
        ret += char2TriBits(i, good);
        if (!good) {
            return ret;
        }
    }

    return ret;
}

auto _toHexString(const std::string &s) -> std::string
{
    std::string ret;
    for (std::string::size_type i = 0; i < s.size(); ++i) {
        std::string tmp = s.substr(i, 4);
        if (tmp.size() == 1) {
            tmp = "000" + tmp;
        } else if (tmp.size() == 2) {
            tmp = "00" + tmp;
        } else if (tmp.size() == 3) {
            tmp = "0" + tmp;
        }

        if (tmp == "0000") {
            ret += '0';
        } else if (tmp == "0001") {
            ret += '0';
        } else if (tmp == "0010") {
            ret += '0';
        } else if (tmp == "0011") {
            ret += '0';
        } else if (tmp == "0100") {
            ret += '0';
        } else if (tmp == "0101") {
            ret += '0';
        } else if (tmp == "0110") {
            ret += '0';
        } else if (tmp == "0111") {
            ret += '0';
        } else if (tmp == "1000") {
            ret += '0';
        } else if (tmp == "1001") {
            ret += '0';
        } else if (tmp == "1010") {
            ret += '0';
        } else if (tmp == "1011") {
            ret += '0';
        } else if (tmp == "1100") {
            ret += '0';
        } else if (tmp == "1101") {
            ret += '0';
        } else if (tmp == "1110") {
            ret += '0';
        } else if (tmp == "1111") {
            ret += '0';
        } else {
            hif_vhdl_standard::hif_vhdl_assert(
                false, "Found unsupported quad bits value: " + s, hif_vhdl_standard::hif_vhdl_error);
        }
    }

    return ret;
}

auto _toOctalString(const std::string &s) -> std::string
{
    std::string ret;
    for (std::string::size_type i = 0; i < s.size(); ++i) {
        std::string tmp = s.substr(i, 3);
        if (tmp.size() == 1) {
            tmp = "00" + tmp;
        } else if (tmp.size() == 2) {
            tmp = "0" + tmp;
        }

        if (tmp == "000") {
            ret += '0';
        } else if (tmp == "001") {
            ret += '0';
        } else if (tmp == "010") {
            ret += '0';
        } else if (tmp == "011") {
            ret += '0';
        } else if (tmp == "100") {
            ret += '0';
        } else if (tmp == "101") {
            ret += '0';
        } else if (tmp == "110") {
            ret += '0';
        } else if (tmp == "111") {
            ret += '0';
        } else {
            hif_vhdl_standard::hif_vhdl_assert(
                false, "Found unsupported quad bits value: " + s, hif_vhdl_standard::hif_vhdl_error);
        }
    }

    return ret;
}

} // namespace

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_logic &value, bool &good)
{
    std::string s            = trim(*l);
    std::string::size_type i = s.find_first_of(" \t");
    s                        = s.substr(0, i);
    if (!_isValidLogic(s)) {
        good = false;
        return;
    }

    *l = l->substr(1);

    char c = _toLogic(s);
    value  = c;

    good = true;
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_logic &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, hdtlib::hl_logic_t &value, bool &good)
{
    sc_dt::sc_logic tmp(value.to_char());
    hif_vhdl_read(l, tmp, good);
    value = tmp.to_char();
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, hdtlib::hl_logic_t &value)
{
    bool good;
    hif_vhdl_read(l, value, good);
}
#endif

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value, bool &good)
{
    std::string s;
    hif_vhdl_std_textio::hif_vhdl_read(l, s, good);
    if (!good) {
        return;
    }
    if (!_isValidLogicVector(s)) {
        good = false;
        return;
    }

    s     = _toLogicVector(s);
    value = s.c_str();

    good = true;
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value, bool &good)
{
    sc_dt::sc_lv<8> v;
    hif_vhdl_read(l, v, good);
    value = static_cast<int8_t>(v.to_int64());
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value, bool &good)
{
    sc_dt::sc_lv<8> v;
    hif_vhdl_read(l, v, good);
    value = static_cast<uint8_t>(v.to_int64());
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value, bool &good)
{
    sc_dt::sc_lv<16> v;
    hif_vhdl_read(l, v, good);
    value = static_cast<int16_t>(v.to_int64());
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value, bool &good)
{
    sc_dt::sc_lv<16> v;
    hif_vhdl_read(l, v, good);
    value = static_cast<uint16_t>(v.to_int64());
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value, bool &good)
{
    sc_dt::sc_lv<32> v;
    hif_vhdl_read(l, v, good);
    value = static_cast<int32_t>(v.to_int64());
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value, bool &good)
{
    sc_dt::sc_lv<32> v;
    hif_vhdl_read(l, v, good);
    value = static_cast<uint32_t>(v.to_int64());
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value, bool &good)
{
    sc_dt::sc_lv<64> v;
    hif_vhdl_read(l, v, good);
    value = v.to_int64();
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value, bool &good)
{
    sc_dt::sc_lv<64> v;
    hif_vhdl_read(l, v, good);
    value = v.to_uint64();
}

void hif_vhdl_read(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_write(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    const sc_dt::sc_logic &value,
    hif_vhdl_std_textio::hif_vhdl_side justified,
    hif_vhdl_std_textio::hif_vhdl_width field)
{
    std::string s;
    s = value.to_char();
    hif_vhdl_std_textio::hif_vhdl_write(l, s, justified, field);
}

void hif_vhdl_write(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    const sc_dt::sc_lv_base &value,
    hif_vhdl_std_textio::hif_vhdl_side justified,
    hif_vhdl_std_textio::hif_vhdl_width field)
{
    std::string s = value.to_string();
    hif_vhdl_std_textio::hif_vhdl_write(l, s, justified, field);
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
void hif_vhdl_write(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    hdtlib::hl_logic_t value,
    hif_vhdl_std_textio::hif_vhdl_side justified,
    hif_vhdl_std_textio::hif_vhdl_width field)
{
    sc_dt::sc_logic tmp(value.to_char());
    hif_vhdl_write(l, tmp, justified, field);
}
#endif

// /////////////////////////////////////////////////////////////////////////////

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value, bool &good)
{
    std::string s;
    hif_vhdl_std_textio::hif_vhdl_read(l, s, good);
    if (!good) {
        return;
    }
    s = string2QuadBits(s, good);
    if (!good) {
        return;
    }

    value = s.c_str();

    good = true;
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value)
{
    bool good = false;
    hif_vhdl_hread(l, value, good);
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value, bool &good)
{
    sc_dt::sc_lv<8> v;
    hif_vhdl_hread(l, v, good);
    value = static_cast<int8_t>(v.to_int64());
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value)
{
    bool good = false;
    hif_vhdl_hread(l, value, good);
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value, bool &good)
{
    sc_dt::sc_lv<8> v;
    hif_vhdl_hread(l, v, good);
    value = static_cast<uint8_t>(v.to_int64());
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value)
{
    bool good = false;
    hif_vhdl_hread(l, value, good);
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value, bool &good)
{
    sc_dt::sc_lv<16> v;
    hif_vhdl_hread(l, v, good);
    value = static_cast<int16_t>(v.to_int64());
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value)
{
    bool good = false;
    hif_vhdl_hread(l, value, good);
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value, bool &good)
{
    sc_dt::sc_lv<16> v;
    hif_vhdl_hread(l, v, good);
    value = static_cast<uint16_t>(v.to_int64());
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value)
{
    bool good = false;
    hif_vhdl_hread(l, value, good);
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value, bool &good)
{
    sc_dt::sc_lv<32> v;
    hif_vhdl_hread(l, v, good);
    value = static_cast<int32_t>(v.to_int64());
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value)
{
    bool good = false;
    hif_vhdl_hread(l, value, good);
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value, bool &good)
{
    sc_dt::sc_lv<32> v;
    hif_vhdl_hread(l, v, good);
    value = static_cast<uint32_t>(v.to_int64());
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value)
{
    bool good = false;
    hif_vhdl_hread(l, value, good);
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value, bool &good)
{
    sc_dt::sc_lv<64> v;
    hif_vhdl_hread(l, v, good);
    value = v.to_int64();
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value)
{
    bool good = false;
    hif_vhdl_hread(l, value, good);
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value, bool &good)
{
    sc_dt::sc_lv<64> v;
    hif_vhdl_read(l, v, good);
    value = v.to_uint64();
}

void hif_vhdl_hread(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_hwrite(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    sc_dt::sc_lv_base value,
    hif_vhdl_std_textio::hif_vhdl_side justified,
    hif_vhdl_std_textio::hif_vhdl_width field)
{
    if (!value.is_01()) {
        value = 'X';
    }
    std::string s = value.to_string();
    s             = _toHexString(s);
    hif_vhdl_std_textio::hif_vhdl_write(l, s, justified, field);
}

// /////////////////////////////////////////////////////////////////////////////

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value, bool &good)
{
    std::string s;
    hif_vhdl_std_textio::hif_vhdl_read(l, s, good);
    if (!good) {
        return;
    }
    s = string2TriBits(s, good);
    if (!good) {
        return;
    }

    value = s.c_str();

    good = true;
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, sc_dt::sc_lv_base &value)
{
    bool good = false;
    hif_vhdl_oread(l, value, good);
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value, bool &good)
{
    sc_dt::sc_lv<8> v;
    hif_vhdl_oread(l, v, good);
    value = static_cast<int8_t>(v.to_int64());
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int8_t &value)
{
    bool good = false;
    hif_vhdl_oread(l, value, good);
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value, bool &good)
{
    sc_dt::sc_lv<8> v;
    hif_vhdl_oread(l, v, good);
    value = static_cast<uint8_t>(v.to_int64());
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint8_t &value)
{
    bool good = false;
    hif_vhdl_oread(l, value, good);
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value, bool &good)
{
    sc_dt::sc_lv<16> v;
    hif_vhdl_oread(l, v, good);
    value = static_cast<int16_t>(v.to_int64());
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int16_t &value)
{
    bool good = false;
    hif_vhdl_oread(l, value, good);
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value, bool &good)
{
    sc_dt::sc_lv<16> v;
    hif_vhdl_oread(l, v, good);
    value = static_cast<uint16_t>(v.to_int64());
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint16_t &value)
{
    bool good = false;
    hif_vhdl_oread(l, value, good);
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value, bool &good)
{
    sc_dt::sc_lv<32> v;
    hif_vhdl_oread(l, v, good);
    value = static_cast<int32_t>(v.to_int64());
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int32_t &value)
{
    bool good = false;
    hif_vhdl_oread(l, value, good);
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value, bool &good)
{
    sc_dt::sc_lv<32> v;
    hif_vhdl_oread(l, v, good);
    value = static_cast<uint32_t>(v.to_int64());
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint32_t &value)
{
    bool good = false;
    hif_vhdl_oread(l, value, good);
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value, bool &good)
{
    sc_dt::sc_lv<64> v;
    hif_vhdl_oread(l, v, good);
    value = v.to_int64();
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, int64_t &value)
{
    bool good = false;
    hif_vhdl_oread(l, value, good);
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value, bool &good)
{
    sc_dt::sc_lv<64> v;
    hif_vhdl_oread(l, v, good);
    value = v.to_uint64();
}

void hif_vhdl_oread(hif_vhdl_std_textio::hif_vhdl_line &l, uint64_t &value)
{
    bool good = false;
    hif_vhdl_oread(l, value, good);
}

void hif_vhdl_owrite(
    hif_vhdl_std_textio::hif_vhdl_line &l,
    sc_dt::sc_lv_base value,
    hif_vhdl_std_textio::hif_vhdl_side justified,
    hif_vhdl_std_textio::hif_vhdl_width field)
{
    if (!value.is_01()) {
        value = '0';
    }
    std::string s = value.to_string();
    s             = _toOctalString(s);
    hif_vhdl_std_textio::hif_vhdl_write(l, s, justified, field);
}

} // namespace hif_vhdl_ieee_std_logic_textio

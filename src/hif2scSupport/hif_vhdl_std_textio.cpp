/// @file hif_vhdl_std_textio.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <cstdio>
#include <cstring>
#include <ios>
#include <sstream>
#include <utility>

#include "hif2scSupport/hif_systemc_extensions.hpp"
#include "hif2scSupport/hif_vhdl_std_textio.hpp"

#include <cmath>

#if (defined _MSC_VER)
#pragma warning(disable : 4127)
#elif defined __clang__
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#elif (defined __GNUC__)
#if __GNUC__ < 7
#pragma GCC diagnostic ignored "-Wattributes"
#endif
#endif

namespace hif_vhdl_std_textio
{

namespace /* anon */
{

void justifyPrint(
    hif_vhdl_line l,
    std::size_t dataSize,
    const hif_vhdl_width field,
    const hif_vhdl_side justified)
{
    if (justified == hif_vhdl_left) {
        for (auto i = static_cast<hif_vhdl_width>(dataSize); i < field; ++i) {
            *l += " ";
        }
    }
}

auto trim(const std::string &str, const std::string &whitespace = " \t") -> std::string
{
    std::size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) {
        return ""; // no content
    }

    std::size_t strEnd   = str.find_last_not_of(whitespace);
    std::size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

} // namespace

HIF2SCSUPPORT_EXPORT
hif_vhdl_text hif_vhdl_input = stdin;

HIF2SCSUPPORT_EXPORT
hif_vhdl_text hif_vhdl_output = stdout;

// Routines to support standard statements.

auto hif_vhdl_file_open(std::string external_name, hif_vhdl_standard::hif_vhdl_file_open_kind open_kind)
    -> hif_vhdl_text
{
    hif_vhdl_text f = nullptr;
    hif_vhdl_file_open(f, std::move(external_name), open_kind);
    return f;
}

//  -- Input Routines for Standard Types

void hif_vhdl_file_open(
    hif_vhdl_text &f,
    const std::string& external_name,
    hif_vhdl_standard::hif_vhdl_file_open_kind open_kind)
{
    hif_vhdl_standard::hif_vhdl_file_open_status s;
    hif_vhdl_file_open(s, f, external_name, open_kind);
}

void hif_vhdl_file_open(
    hif_vhdl_standard::hif_vhdl_file_open_status &status,
    hif_vhdl_text &f,
    const std::string &external_name,
    hif_vhdl_standard::hif_vhdl_file_open_kind open_kind)
{
    const char *type = nullptr;
    switch (open_kind) {
    case hif_vhdl_standard::hif_vhdl_read_mode:
        type = "r";
        break;
    case hif_vhdl_standard::hif_vhdl_write_mode:
        type = "w";
        break;
    case hif_vhdl_standard::hif_vhdl_append_mode:
        type = "a";
        break;
    default:
        assert(false);
        break;
    }
    f = fopen(hif_systemc_extensions::hif_getResourceFileName(external_name).c_str(), type);

    // We are not able to detect errors. Returning general name_error.
    // hif_vhdl__open_ok,
    // hif_vhdl_status_error,
    // hif_vhdl_name_error,
    // hif_vhdl_mode_error
    if (f == nullptr) {
        status = hif_vhdl_standard::hif_vhdl_name_error;
    } else {
        status = hif_vhdl_standard::hif_vhdl__open_ok;
    }
}

void hif_vhdl_file_close(hif_vhdl_text &f) { fclose(f); }

void hif_vhdl_read(hif_vhdl_text &f, std::string &value)
{
    char buffer[256];

    value                = std::string();
    bool continueReading = true;

    do {
        char *res = fgets(buffer, sizeof(buffer), f);
        if (res == nullptr) {
            value = std::string();
            return;
        }

        std::size_t s = std::strlen(buffer);
        continueReading     = (buffer[s - 1] != '\n') && (feof(f) == 0);

        if (buffer[s - 1] == '\n') {
            buffer[s - 1] = '\0';
        }

        value += buffer;
    } while (continueReading);
}

void hif_vhdl_write(hif_vhdl_text &f, const std::string &value) { fprintf(f, "%s", value.c_str()); }

void hif_vhdl_readline(hif_vhdl_text &f, hif_vhdl_line &l)
{
    // C does not have a direct method to get a line...
    // do it manually!
    if (l == nullptr) {
        l = new std::string();
    }
    l->clear();
    if (f == nullptr) {
        std::cerr << "ERROR: file argument is nullptr.\n";
        assert(false);
        exit(1);
    }
    int c = fgetc(f);
    while (c != EOF && c != '\n') {
        const char ch = static_cast<char>(c);
        l->push_back(ch);
        c = fgetc(f);
    }
}

void hif_vhdl_read(hif_vhdl_line &l, sc_dt::sc_bit &value, bool &good)
{
    std::string s            = trim(*l);
    std::string::size_type i = s.find_first_of(" \t");
    s                        = s.substr(0, i);
    if (s.size() != 1 && s != "1" && s != "0") {
        good = false;
        return;
    }

    *l = l->substr(1);

    if (s == "1") {
        value = '1';
    } else {
        value = '0';
    }

    good = true;
}

void hif_vhdl_read(hif_vhdl_line &l, sc_dt::sc_bit &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_line &l, sc_dt::sc_bv_base &value, bool &good)
{
    std::string s            = trim(*l);
    std::string::size_type i = s.find_first_of(" \t");
    s                        = s.substr(0, i);
    if (s.empty() || s.find_first_not_of("01") != std::string::npos) {
        good = false;
        return;
    }

    *l = l->substr(s.size());

    value = s.c_str();

    good = true;
}

void hif_vhdl_read(hif_vhdl_line &l, sc_dt::sc_bv_base &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_line &l, bool &value, bool &good)
{
    std::string s            = trim(*l);
    std::string::size_type i = s.find_first_of(" \t");
    s                        = s.substr(0, i);
    if (s != "TRUE" && s != "FALSE") {
        good = false;
        return;
    }

    *l = l->substr(s.size());

    value = s == "TRUE";

    good = true;
}

void hif_vhdl_read(hif_vhdl_line &l, bool &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_line &l, char &value, bool &good)
{
    std::string s            = trim(*l);
    std::string::size_type i = s.find_first_of(" \t");
    s                        = s.substr(0, i);
    if (s.size() != 1) {
        good = false;
        return;
    }

    *l = l->substr(1);

    value = s[0];

    good = true;
}

void hif_vhdl_read(hif_vhdl_line &l, char &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_line &l, int32_t &value, bool &good)
{
    std::string s            = trim(*l);
    std::string::size_type i = s.find_first_of(" \t");
    s                        = s.substr(0, i);

    std::stringstream ss;
    ss << s;
    ss >> value;

    if (!ss.str().empty()) {
        good = false;
        return;
    }

    good = true;
}

void hif_vhdl_read(hif_vhdl_line &l, int32_t &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_line &l, double &value, bool &good)
{
    std::string s            = trim(*l);
    std::string::size_type i = s.find_first_of(" \t");
    s                        = s.substr(0, i);

    std::stringstream ss;
    ss << s;
    ss >> value;

    if (!ss.str().empty()) {
        good = false;
        return;
    }

    good = true;
}

void hif_vhdl_read(hif_vhdl_line &l, double &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_line &l, std::string &value, bool &good)
{
    std::string s            = trim(*l);
    std::string::size_type i = s.find_first_of(" \t");
    s                        = s.substr(0, i);

    std::stringstream ss;
    ss << s;
    ss >> value;

    if (ss.str().empty()) {
        good = false;
        return;
    }

    good = true;
}

void hif_vhdl_read(hif_vhdl_line &l, std::string &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

void hif_vhdl_read(hif_vhdl_line &l, sc_core::sc_time &value, bool &good)
{
    double d = NAN;
    std::string unit;
    std::string restore = *l;
    hif_vhdl_read(l, d, good);
    if (!good) {
        *l   = restore;
        good = false;
        return;
    }
    hif_vhdl_read(l, unit, good);
    if (!good) {
        *l   = restore;
        good = false;
        return;
    }

    sc_core::sc_time_unit u;
    if (unit == "fs") {
        u = sc_core::SC_FS;
    } else if (unit == "ps") {
        u = sc_core::SC_PS;
    } else if (unit == "ns") {
        u = sc_core::SC_NS;
    } else if (unit == "us") {
        u = sc_core::SC_US;
    } else if (unit == "ms") {
        u = sc_core::SC_MS;
    } else if (unit == "sec") {
        u = sc_core::SC_SEC;
    } else {
        *l   = restore;
        good = false;
        return;
    }

    value = sc_core::sc_time(d, u);
    good  = true;
}

void hif_vhdl_read(hif_vhdl_line &l, sc_core::sc_time &value)
{
    bool good = false;
    hif_vhdl_read(l, value, good);
}

//  -- Output Routines for Standard Types

void hif_vhdl_writeline(hif_vhdl_text &f, hif_vhdl_line &l)
{
    fprintf(f, "%s", l->c_str());
    l->clear();
}

void hif_vhdl_write(hif_vhdl_line &l, const sc_dt::sc_bit &value, hif_vhdl_side justified, hif_vhdl_width field)
{
    if (!l->empty()) {
        *l += " ";
    }

    justifyPrint(l, 1, field, justified);

    if (value == '1') {
        *l += "1";
    } else {
        *l += "0";
    }

    justifyPrint(l, 1, field, justified);
}

void hif_vhdl_write(hif_vhdl_line &l, const sc_dt::sc_bv_base &value, hif_vhdl_side justified, hif_vhdl_width field)
{
    if (!l->empty()) {
        *l += " ";
    }

    justifyPrint(l, value.to_string().size(), field, justified);

    *l += value.to_string();

    justifyPrint(l, value.to_string().size(), field, justified);
}

void hif_vhdl_write(hif_vhdl_line &l, bool value, hif_vhdl_side justified, hif_vhdl_width field)
{
    if (!l->empty()) {
        *l += " ";
    }

    justifyPrint(l, value ? 4 : 5, field, justified);

    if (value) {
        *l += "TRUE";
    } else {
        *l += "FALSE";
    }

    justifyPrint(l, 1, field, justified);
}

void hif_vhdl_write(hif_vhdl_line &l, char value, hif_vhdl_side justified, hif_vhdl_width field)
{
    if (!l->empty()) {
        *l += " ";
    }

    justifyPrint(l, 1, field, justified);

    *l += value;

    justifyPrint(l, 1, field, justified);
}

void hif_vhdl_write(hif_vhdl_line &l, int32_t value, hif_vhdl_side justified, hif_vhdl_width field)
{
    if (!l->empty()) {
        *l += " ";
    }

    std::stringstream ss;
    ss << value;

    justifyPrint(l, ss.str().size(), field, justified);

    *l += ss.str();

    justifyPrint(l, ss.str().size(), field, justified);
}

void hif_vhdl_write(hif_vhdl_line &l, int32_t value, hif_vhdl_side justified, hif_vhdl_width field, uint32_t digits)
{
    if (!l->empty()) {
        *l += " ";
    }

    std::stringstream ss;
    if (digits == 0) {
        ss << std::scientific;
    } else {
        ss.precision(static_cast<std::streamsize>(digits));
        ss << std::fixed;
    }

    ss << value;
    std::string s;
    ss >> s;

    if (digits != 0) {
        s = s.substr(0, s.find('.') + digits);
    }

    justifyPrint(l, s.size(), field, justified);

    *l += s;

    justifyPrint(l, s.size(), field, justified);
}

void hif_vhdl_write(hif_vhdl_line &l, const std::string &value, hif_vhdl_side justified, hif_vhdl_width field)
{
    if (!l->empty()) {
        *l += " ";
    }

    justifyPrint(l, value.size(), field, justified);

    *l += value;

    justifyPrint(l, value.size(), field, justified);
}

void hif_vhdl_write(
    hif_vhdl_line &l,
    sc_core::sc_time value,
    hif_vhdl_side justified,
    hif_vhdl_width field,
    sc_core::sc_time_unit unit)
{
    double t = value.to_seconds();
    switch (unit) {
    case sc_core::SC_FS:
        t = t / 1000.0;
        [[fallthrough]];
    case sc_core::SC_PS:
        t = t / 1000.0;
        [[fallthrough]];
    case sc_core::SC_NS:
        t = t / 1000.0;
        [[fallthrough]];
    case sc_core::SC_US:
        t = t / 1000.0;
        [[fallthrough]];
    case sc_core::SC_MS:
        t = t / 1000.0;
        [[fallthrough]];
    case sc_core::SC_SEC:
        break;
    default:
        std::cerr << "Invalid time unit.\n";
        exit(1);
    }

    std::stringstream ss;
    ss << t << " ";
    switch (unit) {
    case sc_core::SC_FS:
        ss << "fs";
        break;
    case sc_core::SC_PS:
        ss << "ps";
        break;
    case sc_core::SC_NS:
        ss << "ns";
        break;
    case sc_core::SC_US:
        ss << "us";
        break;
    case sc_core::SC_MS:
        ss << "ms";
        break;
    case sc_core::SC_SEC:
        ss << "sec";
        break;
    default:
        std::cerr << "Invalid time unit.\n";
        exit(1);
    }

    std::string v = ss.str();

    hif_vhdl_write(l, v, justified, field);
}

//  -- File Position Predicates

auto hif_vhdl_endfile(hif_vhdl_text f) -> bool { return (feof(f) != 0); }

} // namespace hif_vhdl_std_textio

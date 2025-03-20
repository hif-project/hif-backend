/// @file hif_verilog_standard.i.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "../hif_systemc_extensions.hpp"
#include "../hif_verilog_standard.hpp"

#include <cassert>
#include <fstream>
#include <iomanip>

namespace hif_verilog_standard
{

template <int times, int size>
sc_dt::sc_lv<times * size> hif_verilog_iterated_concat(sc_dt::sc_lv<size> expression)
{
    sc_dt::sc_lv<times * size> result;
    unsigned int res_i = 0;
    for (int i = 0; i < times; ++i)
        for (int j = 0; j < size; ++j, ++res_i)
            result[res_i] = expression[j];
    return result;
}

template <size_t L, int W>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    sc_dt::sc_bv<W> (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        // normal cell case:
        assert(str.size() == W);

        if ((step > 0 && n > stopAddr) || (step < 0 && n < stopAddr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = sc_dt::sc_bv<W>(str.c_str());
        n += step;
    }
}

template <size_t L, int W>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    sc_dt::sc_lv<W> (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        // normal cell case:
        assert(str.size() == W);

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = sc_dt::sc_lv<W>(str.c_str());
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    uint8_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = sc_dt::sc_lv<8>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    uint16_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = sc_dt::sc_lv<16>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    uint32_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = sc_dt::sc_lv<32>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    uint64_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = sc_dt::sc_lv<64>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    int8_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = sc_dt::sc_lv<8>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    int16_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = sc_dt::sc_lv<16>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    int32_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = sc_dt::sc_lv<32>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    int64_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = sc_dt::sc_lv<64>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    bool (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = sc_dt::sc_lv<64>(str.c_str()).to_uint64() == 1;
        n += step;
    }
}

template <size_t L, int W>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    sc_dt::sc_bv<W> (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        // normal cell case:
        assert(str.size() == W / 4);

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = sc_dt::sc_bv<W>(str.c_str());
        n += step;
    }
}

template <size_t L, int W>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    sc_dt::sc_lv<W> (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        // normal cell case:
        assert(str.size() == W / 4);

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = sc_dt::sc_lv<W>(str.c_str());
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    uint8_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = sc_dt::sc_lv<8>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    uint16_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = sc_dt::sc_lv<16>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    uint32_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = sc_dt::sc_lv<32>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    uint64_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = sc_dt::sc_lv<64>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    int8_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = sc_dt::sc_lv<8>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    int16_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = sc_dt::sc_lv<16>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    int32_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = sc_dt::sc_lv<32>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    int64_t (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = sc_dt::sc_lv<64>(str.c_str()).to_uint64();
        n += step;
    }
}

template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    bool (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;
            in >> str;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = sc_dt::sc_lv<64>(str.c_str()).to_uint64() == 1;
        n += step;
    }
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int times, int size>
hdtlib::hl_lv_t<times * size> hif_verilog_iterated_concat(hdtlib::hl_lv_t<size> expression)
{
    hdtlib::hl_lv_t<times * size> result;
    unsigned int res_i = 0;
    for (int i = 0; i < times; ++i)
        for (int j = 0; j < size; ++j, ++res_i)
            result.set_bit(res_i, expression[j]);
    return result;
}

template <size_t L, int W>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    hdtlib::hl_bv_t<W> (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        // normal cell case:
        assert(str.size() == W);

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = hdtlib::hl_bv_t<W>(str.c_str());
        n += step;
    }
}

template <size_t L, int W>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    hdtlib::hl_lv_t<W> (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        // normal cell case:
        assert(str.size() == W);

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        memory_name[n] = hdtlib::hl_lv_t<W>(str.c_str());
        n += step;
    }
}

template <size_t L, int W>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    hdtlib::hl_bv_t<W> (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        // normal cell case:
        assert(str.size() == W / 4);

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = hdtlib::hl_bv_t<W>(str.c_str());
        n += step;
    }
}

template <size_t L, int W>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    hdtlib::hl_lv_t<W> (&memory_name)[L],
    const int start_addr,
    const int stop_addr,
    const int min,
    const int direction)
{
    std::ifstream in(hif_systemc_extensions::hif_getResourceFileName(file_name).c_str());
    assert(in.good());

    int startAddr = start_addr - min;
    if (direction < 0)
        startAddr = L - startAddr;
    int stopAddr = stop_addr - min;
    if (direction < 0)
        stopAddr = L - stopAddr;

    int n          = startAddr;
    const int step = startAddr <= stopAddr ? 1 : -1;

    while (in.good()) {
        std::string str;
        in >> str;
        if (!in.good())
            break;

        if (str[0] == '@') {
            std::stringstream ss;
            ss << str.substr(1);
            ss << std::hex;
            ss >> n;

            n = n - min;
            if (direction < 0)
                n = L - n;
        }

        // normal cell case:
        assert(str.size() == W / 4);

        if ((step > 0 && n > stop_addr) || (step < 0 && n < stop_addr))
            break;
        if (n > L - 1)
            break;

        str            = "0x" + str;
        memory_name[n] = hdtlib::hl_lv_t<W>(str.c_str());
        n += step;
    }
}

#endif

} // namespace hif_verilog_standard

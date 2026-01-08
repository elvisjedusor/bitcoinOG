// Copyright (c) 2017-present The Bitcoin Core developers
// Distributed under the MIT software license

#ifndef BITOK_CRYPTO_CPUID_H
#define BITOK_CRYPTO_CPUID_H

#include <cstdint>

#if defined(__x86_64__) || defined(__amd64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
#define HAVE_GETCPUID

#ifdef _MSC_VER
#include <intrin.h>
inline void GetCPUID(uint32_t leaf, uint32_t subleaf, uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d)
{
    int regs[4];
    __cpuidex(regs, leaf, subleaf);
    a = regs[0];
    b = regs[1];
    c = regs[2];
    d = regs[3];
}
#else
#include <cpuid.h>
inline void GetCPUID(uint32_t leaf, uint32_t subleaf, uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d)
{
    __cpuid_count(leaf, subleaf, a, b, c, d);
}
#endif

#endif

#endif // BITOK_CRYPTO_CPUID_H

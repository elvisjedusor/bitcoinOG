// Copyright (c) 2018-present The Bitcoin Core developers
// Adapted for Bitok - SHA-NI hardware acceleration
// Distributed under the MIT software license
//
// Based on https://github.com/noloader/SHA-Intrinsics/blob/master/sha256-x86.c
// Written and placed in public domain by Jeffrey Walton
// Based on code from Intel, and by Sean Gulley for the miTLS project

#if defined(ENABLE_X86_SHANI) && (defined(__x86_64__) || defined(__amd64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86))

#include <cstdint>
#include <cstring>
#include <immintrin.h>

namespace {

alignas(16) const uint8_t MASK[16] = {
    0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04,
    0x0b, 0x0a, 0x09, 0x08, 0x0f, 0x0e, 0x0d, 0x0c
};

alignas(16) const uint8_t INIT0[16] = {
    0x8c, 0x68, 0x05, 0x9b, 0x7f, 0x52, 0x0e, 0x51,
    0x85, 0xae, 0x67, 0xbb, 0x67, 0xe6, 0x09, 0x6a
};

alignas(16) const uint8_t INIT1[16] = {
    0x19, 0xcd, 0xe0, 0x5b, 0xab, 0xd9, 0x83, 0x1f,
    0x3a, 0xf5, 0x4f, 0xa5, 0x72, 0xf3, 0x6e, 0x3c
};

inline void QuadRound(__m128i& state0, __m128i& state1, uint64_t k1, uint64_t k0)
{
    const __m128i msg = _mm_set_epi64x(k1, k0);
    state1 = _mm_sha256rnds2_epu32(state1, state0, msg);
    state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(msg, 0x0e));
}

inline void QuadRound(__m128i& state0, __m128i& state1, __m128i m, uint64_t k1, uint64_t k0)
{
    const __m128i msg = _mm_add_epi32(m, _mm_set_epi64x(k1, k0));
    state1 = _mm_sha256rnds2_epu32(state1, state0, msg);
    state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(msg, 0x0e));
}

inline void ShiftMessageA(__m128i& m0, __m128i m1)
{
    m0 = _mm_sha256msg1_epu32(m0, m1);
}

inline void ShiftMessageC(__m128i& m0, __m128i m1, __m128i& m2)
{
    m2 = _mm_sha256msg2_epu32(_mm_add_epi32(m2, _mm_alignr_epi8(m1, m0, 4)), m1);
}

inline void ShiftMessageB(__m128i& m0, __m128i m1, __m128i& m2)
{
    ShiftMessageC(m0, m1, m2);
    ShiftMessageA(m0, m1);
}

inline void Shuffle(__m128i& s0, __m128i& s1)
{
    const __m128i t1 = _mm_shuffle_epi32(s0, 0xB1);
    const __m128i t2 = _mm_shuffle_epi32(s1, 0x1B);
    s0 = _mm_alignr_epi8(t1, t2, 0x08);
    s1 = _mm_blend_epi16(t2, t1, 0xF0);
}

inline void Unshuffle(__m128i& s0, __m128i& s1)
{
    const __m128i t1 = _mm_shuffle_epi32(s0, 0x1B);
    const __m128i t2 = _mm_shuffle_epi32(s1, 0xB1);
    s0 = _mm_blend_epi16(t1, t2, 0xF0);
    s1 = _mm_alignr_epi8(t2, t1, 0x08);
}

inline __m128i Load(const unsigned char* in)
{
    return _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)in), _mm_load_si128((const __m128i*)MASK));
}

inline void Save(unsigned char* out, __m128i s)
{
    _mm_storeu_si128((__m128i*)out, _mm_shuffle_epi8(s, _mm_load_si128((const __m128i*)MASK)));
}

}

namespace sha256_x86_shani {

void Transform(uint32_t* s, const unsigned char* chunk, size_t blocks)
{
    __m128i m0, m1, m2, m3, s0, s1, so0, so1;

    s0 = _mm_loadu_si128((const __m128i*)s);
    s1 = _mm_loadu_si128((const __m128i*)(s + 4));
    Shuffle(s0, s1);

    while (blocks--) {
        so0 = s0;
        so1 = s1;

        m0 = Load(chunk);
        QuadRound(s0, s1, m0, 0xe9b5dba5b5c0fbcfull, 0x71374491428a2f98ull);
        m1 = Load(chunk + 16);
        QuadRound(s0, s1, m1, 0xab1c5ed5923f82a4ull, 0x59f111f13956c25bull);
        ShiftMessageA(m0, m1);
        m2 = Load(chunk + 32);
        QuadRound(s0, s1, m2, 0x550c7dc3243185beull, 0x12835b01d807aa98ull);
        ShiftMessageA(m1, m2);
        m3 = Load(chunk + 48);
        QuadRound(s0, s1, m3, 0xc19bf1749bdc06a7ull, 0x80deb1fe72be5d74ull);
        ShiftMessageB(m2, m3, m0);
        QuadRound(s0, s1, m0, 0x240ca1cc0fc19dc6ull, 0xefbe4786e49b69c1ull);
        ShiftMessageB(m3, m0, m1);
        QuadRound(s0, s1, m1, 0x76f988da5cb0a9dcull, 0x4a7484aa2de92c6full);
        ShiftMessageB(m0, m1, m2);
        QuadRound(s0, s1, m2, 0xbf597fc7b00327c8ull, 0xa831c66d983e5152ull);
        ShiftMessageB(m1, m2, m3);
        QuadRound(s0, s1, m3, 0x1429296706ca6351ull, 0xd5a79147c6e00bf3ull);
        ShiftMessageB(m2, m3, m0);
        QuadRound(s0, s1, m0, 0x53380d134d2c6dfcull, 0x2e1b213827b70a85ull);
        ShiftMessageB(m3, m0, m1);
        QuadRound(s0, s1, m1, 0x92722c8581c2c92eull, 0x766a0abb650a7354ull);
        ShiftMessageB(m0, m1, m2);
        QuadRound(s0, s1, m2, 0xc76c51a3c24b8b70ull, 0xa81a664ba2bfe8a1ull);
        ShiftMessageB(m1, m2, m3);
        QuadRound(s0, s1, m3, 0x106aa070f40e3585ull, 0xd6990624d192e819ull);
        ShiftMessageB(m2, m3, m0);
        QuadRound(s0, s1, m0, 0x34b0bcb52748774cull, 0x1e376c0819a4c116ull);
        ShiftMessageB(m3, m0, m1);
        QuadRound(s0, s1, m1, 0x682e6ff35b9cca4full, 0x4ed8aa4a391c0cb3ull);
        ShiftMessageC(m0, m1, m2);
        QuadRound(s0, s1, m2, 0x8cc7020884c87814ull, 0x78a5636f748f82eeull);
        ShiftMessageC(m1, m2, m3);
        QuadRound(s0, s1, m3, 0xc67178f2bef9a3f7ull, 0xa4506ceb90befffaull);

        s0 = _mm_add_epi32(s0, so0);
        s1 = _mm_add_epi32(s1, so1);

        chunk += 64;
    }

    Unshuffle(s0, s1);
    _mm_storeu_si128((__m128i*)s, s0);
    _mm_storeu_si128((__m128i*)(s + 4), s1);
}

void TransformD64(unsigned char* out, const unsigned char* in)
{
    __m128i m0, m1, m2, m3, s0, s1, so0, so1;

    s0 = _mm_load_si128((const __m128i*)INIT0);
    s1 = _mm_load_si128((const __m128i*)INIT1);

    m0 = Load(in);
    QuadRound(s0, s1, m0, 0xe9b5dba5b5c0fbcfull, 0x71374491428a2f98ull);
    m1 = Load(in + 16);
    QuadRound(s0, s1, m1, 0xab1c5ed5923f82a4ull, 0x59f111f13956c25bull);
    ShiftMessageA(m0, m1);
    m2 = Load(in + 32);
    QuadRound(s0, s1, m2, 0x550c7dc3243185beull, 0x12835b01d807aa98ull);
    ShiftMessageA(m1, m2);
    m3 = Load(in + 48);
    QuadRound(s0, s1, m3, 0xc19bf1749bdc06a7ull, 0x80deb1fe72be5d74ull);
    ShiftMessageB(m2, m3, m0);
    QuadRound(s0, s1, m0, 0x240ca1cc0fc19dc6ull, 0xefbe4786e49b69c1ull);
    ShiftMessageB(m3, m0, m1);
    QuadRound(s0, s1, m1, 0x76f988da5cb0a9dcull, 0x4a7484aa2de92c6full);
    ShiftMessageB(m0, m1, m2);
    QuadRound(s0, s1, m2, 0xbf597fc7b00327c8ull, 0xa831c66d983e5152ull);
    ShiftMessageB(m1, m2, m3);
    QuadRound(s0, s1, m3, 0x1429296706ca6351ull, 0xd5a79147c6e00bf3ull);
    ShiftMessageB(m2, m3, m0);
    QuadRound(s0, s1, m0, 0x53380d134d2c6dfcull, 0x2e1b213827b70a85ull);
    ShiftMessageB(m3, m0, m1);
    QuadRound(s0, s1, m1, 0x92722c8581c2c92eull, 0x766a0abb650a7354ull);
    ShiftMessageB(m0, m1, m2);
    QuadRound(s0, s1, m2, 0xc76c51a3c24b8b70ull, 0xa81a664ba2bfe8a1ull);
    ShiftMessageB(m1, m2, m3);
    QuadRound(s0, s1, m3, 0x106aa070f40e3585ull, 0xd6990624d192e819ull);
    ShiftMessageB(m2, m3, m0);
    QuadRound(s0, s1, m0, 0x34b0bcb52748774cull, 0x1e376c0819a4c116ull);
    ShiftMessageB(m3, m0, m1);
    QuadRound(s0, s1, m1, 0x682e6ff35b9cca4full, 0x4ed8aa4a391c0cb3ull);
    ShiftMessageC(m0, m1, m2);
    QuadRound(s0, s1, m2, 0x8cc7020884c87814ull, 0x78a5636f748f82eeull);
    ShiftMessageC(m1, m2, m3);
    QuadRound(s0, s1, m3, 0xc67178f2bef9a3f7ull, 0xa4506ceb90befffaull);
    s0 = _mm_add_epi32(s0, _mm_load_si128((const __m128i*)INIT0));
    s1 = _mm_add_epi32(s1, _mm_load_si128((const __m128i*)INIT1));

    so0 = s0;
    so1 = s1;
    QuadRound(s0, s1, 0xe9b5dba5b5c0fbcfull, 0x71374491c28a2f98ull);
    QuadRound(s0, s1, 0xab1c5ed5923f82a4ull, 0x59f111f13956c25bull);
    QuadRound(s0, s1, 0x550c7dc3243185beull, 0x12835b01d807aa98ull);
    QuadRound(s0, s1, 0xc19bf3749bdc06a7ull, 0x80deb1fe72be5d74ull);
    QuadRound(s0, s1, 0x240cf2540fe1edc6ull, 0xf0fe4786649b69c1ull);
    QuadRound(s0, s1, 0x16f988fa61b9411eull, 0x6cc984be4fe9346full);
    QuadRound(s0, s1, 0xb9d99ec7b019fc65ull, 0xa88e5a6df2c65152ull);
    QuadRound(s0, s1, 0xc7353eb0fdb1232bull, 0xe70eeaa09a1231c3ull);
    QuadRound(s0, s1, 0xdc1eeefd5a0f118full, 0xcb976d5f3069bad5ull);
    QuadRound(s0, s1, 0xe15d5b1658f4ca9dull, 0xde0b7a040a35b689ull);
    QuadRound(s0, s1, 0x6fab9537a507ea32ull, 0x37088980007f3e86ull);
    QuadRound(s0, s1, 0xc0bbbe37cdaa3b6dull, 0x0d8cd6f117406110ull);
    QuadRound(s0, s1, 0x6fd15ca70b02e931ull, 0xdb48a36383613bdaull);
    QuadRound(s0, s1, 0x6d4378906ed41a95ull, 0x31338431521afacaull);
    QuadRound(s0, s1, 0x532fb63cb5c9a0e6ull, 0x9eccabbdc39c91f2ull);
    QuadRound(s0, s1, 0x4c191d76a4954b68ull, 0x07237ea3d2c741c6ull);
    s0 = _mm_add_epi32(s0, so0);
    s1 = _mm_add_epi32(s1, so1);

    Unshuffle(s0, s1);
    m0 = s0;
    m1 = s1;

    s0 = _mm_load_si128((const __m128i*)INIT0);
    s1 = _mm_load_si128((const __m128i*)INIT1);
    QuadRound(s0, s1, m0, 0xe9b5dba5b5c0fbcfull, 0x71374491428a2f98ull);
    QuadRound(s0, s1, m1, 0xab1c5ed5923f82a4ull, 0x59f111f13956c25bull);
    ShiftMessageA(m0, m1);
    m2 = _mm_set_epi64x(0x0ull, 0x80000000ull);
    QuadRound(s0, s1, 0x550c7dc3243185beull, 0x12835b015807aa98ull);
    ShiftMessageA(m1, m2);
    m3 = _mm_set_epi64x(0x10000000000ull, 0x0ull);
    QuadRound(s0, s1, 0xc19bf2749bdc06a7ull, 0x80deb1fe72be5d74ull);
    ShiftMessageB(m2, m3, m0);
    QuadRound(s0, s1, m0, 0x240ca1cc0fc19dc6ull, 0xefbe4786e49b69c1ull);
    ShiftMessageB(m3, m0, m1);
    QuadRound(s0, s1, m1, 0x76f988da5cb0a9dcull, 0x4a7484aa2de92c6full);
    ShiftMessageB(m0, m1, m2);
    QuadRound(s0, s1, m2, 0xbf597fc7b00327c8ull, 0xa831c66d983e5152ull);
    ShiftMessageB(m1, m2, m3);
    QuadRound(s0, s1, m3, 0x1429296706ca6351ull, 0xd5a79147c6e00bf3ull);
    ShiftMessageB(m2, m3, m0);
    QuadRound(s0, s1, m0, 0x53380d134d2c6dfcull, 0x2e1b213827b70a85ull);
    ShiftMessageB(m3, m0, m1);
    QuadRound(s0, s1, m1, 0x92722c8581c2c92eull, 0x766a0abb650a7354ull);
    ShiftMessageB(m0, m1, m2);
    QuadRound(s0, s1, m2, 0xc76c51a3c24b8b70ull, 0xa81a664ba2bfe8a1ull);
    ShiftMessageB(m1, m2, m3);
    QuadRound(s0, s1, m3, 0x106aa070f40e3585ull, 0xd6990624d192e819ull);
    ShiftMessageB(m2, m3, m0);
    QuadRound(s0, s1, m0, 0x34b0bcb52748774cull, 0x1e376c0819a4c116ull);
    ShiftMessageB(m3, m0, m1);
    QuadRound(s0, s1, m1, 0x682e6ff35b9cca4full, 0x4ed8aa4a391c0cb3ull);
    ShiftMessageC(m0, m1, m2);
    QuadRound(s0, s1, m2, 0x8cc7020884c87814ull, 0x78a5636f748f82eeull);
    ShiftMessageC(m1, m2, m3);
    QuadRound(s0, s1, m3, 0xc67178f2bef9a3f7ull, 0xa4506ceb90befffaull);
    s0 = _mm_add_epi32(s0, _mm_load_si128((const __m128i*)INIT0));
    s1 = _mm_add_epi32(s1, _mm_load_si128((const __m128i*)INIT1));

    Unshuffle(s0, s1);
    Save(out, s0);
    Save(out + 16, s1);
}

}

#endif

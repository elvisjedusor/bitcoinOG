// Copyright (c) 2014-present The Bitcoin Core developers
// Adapted for Bitok - optimized SHA256 implementation
// Distributed under the MIT software license

#include "sha256.h"
#include "cpuid.h"

#include <cstring>
#include <cassert>

namespace {

const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) { return z ^ (x & (y ^ z)); }
inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (z & (x | y)); }
inline uint32_t Sigma0(uint32_t x) { return (x >> 2 | x << 30) ^ (x >> 13 | x << 19) ^ (x >> 22 | x << 10); }
inline uint32_t Sigma1(uint32_t x) { return (x >> 6 | x << 26) ^ (x >> 11 | x << 21) ^ (x >> 25 | x << 7); }
inline uint32_t sigma0(uint32_t x) { return (x >> 7 | x << 25) ^ (x >> 18 | x << 14) ^ (x >> 3); }
inline uint32_t sigma1(uint32_t x) { return (x >> 17 | x << 15) ^ (x >> 19 | x << 13) ^ (x >> 10); }

inline uint32_t ReadBE32(const unsigned char* ptr)
{
    return (uint32_t(ptr[0]) << 24) | (uint32_t(ptr[1]) << 16) | (uint32_t(ptr[2]) << 8) | uint32_t(ptr[3]);
}

inline void WriteBE32(unsigned char* ptr, uint32_t x)
{
    ptr[0] = x >> 24;
    ptr[1] = x >> 16;
    ptr[2] = x >> 8;
    ptr[3] = x;
}

inline void Round(uint32_t a, uint32_t b, uint32_t c, uint32_t& d, uint32_t e, uint32_t f, uint32_t g, uint32_t& h, uint32_t k)
{
    uint32_t t1 = h + Sigma1(e) + Ch(e, f, g) + k;
    uint32_t t2 = Sigma0(a) + Maj(a, b, c);
    d += t1;
    h = t1 + t2;
}

void Transform_C(uint32_t* s, const unsigned char* chunk, size_t blocks)
{
    while (blocks--) {
        uint32_t a = s[0], b = s[1], c = s[2], d = s[3], e = s[4], f = s[5], g = s[6], h = s[7];
        uint32_t w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15;

        Round(a, b, c, d, e, f, g, h, K[0] + (w0 = ReadBE32(chunk + 0)));
        Round(h, a, b, c, d, e, f, g, K[1] + (w1 = ReadBE32(chunk + 4)));
        Round(g, h, a, b, c, d, e, f, K[2] + (w2 = ReadBE32(chunk + 8)));
        Round(f, g, h, a, b, c, d, e, K[3] + (w3 = ReadBE32(chunk + 12)));
        Round(e, f, g, h, a, b, c, d, K[4] + (w4 = ReadBE32(chunk + 16)));
        Round(d, e, f, g, h, a, b, c, K[5] + (w5 = ReadBE32(chunk + 20)));
        Round(c, d, e, f, g, h, a, b, K[6] + (w6 = ReadBE32(chunk + 24)));
        Round(b, c, d, e, f, g, h, a, K[7] + (w7 = ReadBE32(chunk + 28)));
        Round(a, b, c, d, e, f, g, h, K[8] + (w8 = ReadBE32(chunk + 32)));
        Round(h, a, b, c, d, e, f, g, K[9] + (w9 = ReadBE32(chunk + 36)));
        Round(g, h, a, b, c, d, e, f, K[10] + (w10 = ReadBE32(chunk + 40)));
        Round(f, g, h, a, b, c, d, e, K[11] + (w11 = ReadBE32(chunk + 44)));
        Round(e, f, g, h, a, b, c, d, K[12] + (w12 = ReadBE32(chunk + 48)));
        Round(d, e, f, g, h, a, b, c, K[13] + (w13 = ReadBE32(chunk + 52)));
        Round(c, d, e, f, g, h, a, b, K[14] + (w14 = ReadBE32(chunk + 56)));
        Round(b, c, d, e, f, g, h, a, K[15] + (w15 = ReadBE32(chunk + 60)));

        Round(a, b, c, d, e, f, g, h, K[16] + (w0 += sigma1(w14) + w9 + sigma0(w1)));
        Round(h, a, b, c, d, e, f, g, K[17] + (w1 += sigma1(w15) + w10 + sigma0(w2)));
        Round(g, h, a, b, c, d, e, f, K[18] + (w2 += sigma1(w0) + w11 + sigma0(w3)));
        Round(f, g, h, a, b, c, d, e, K[19] + (w3 += sigma1(w1) + w12 + sigma0(w4)));
        Round(e, f, g, h, a, b, c, d, K[20] + (w4 += sigma1(w2) + w13 + sigma0(w5)));
        Round(d, e, f, g, h, a, b, c, K[21] + (w5 += sigma1(w3) + w14 + sigma0(w6)));
        Round(c, d, e, f, g, h, a, b, K[22] + (w6 += sigma1(w4) + w15 + sigma0(w7)));
        Round(b, c, d, e, f, g, h, a, K[23] + (w7 += sigma1(w5) + w0 + sigma0(w8)));
        Round(a, b, c, d, e, f, g, h, K[24] + (w8 += sigma1(w6) + w1 + sigma0(w9)));
        Round(h, a, b, c, d, e, f, g, K[25] + (w9 += sigma1(w7) + w2 + sigma0(w10)));
        Round(g, h, a, b, c, d, e, f, K[26] + (w10 += sigma1(w8) + w3 + sigma0(w11)));
        Round(f, g, h, a, b, c, d, e, K[27] + (w11 += sigma1(w9) + w4 + sigma0(w12)));
        Round(e, f, g, h, a, b, c, d, K[28] + (w12 += sigma1(w10) + w5 + sigma0(w13)));
        Round(d, e, f, g, h, a, b, c, K[29] + (w13 += sigma1(w11) + w6 + sigma0(w14)));
        Round(c, d, e, f, g, h, a, b, K[30] + (w14 += sigma1(w12) + w7 + sigma0(w15)));
        Round(b, c, d, e, f, g, h, a, K[31] + (w15 += sigma1(w13) + w8 + sigma0(w0)));

        Round(a, b, c, d, e, f, g, h, K[32] + (w0 += sigma1(w14) + w9 + sigma0(w1)));
        Round(h, a, b, c, d, e, f, g, K[33] + (w1 += sigma1(w15) + w10 + sigma0(w2)));
        Round(g, h, a, b, c, d, e, f, K[34] + (w2 += sigma1(w0) + w11 + sigma0(w3)));
        Round(f, g, h, a, b, c, d, e, K[35] + (w3 += sigma1(w1) + w12 + sigma0(w4)));
        Round(e, f, g, h, a, b, c, d, K[36] + (w4 += sigma1(w2) + w13 + sigma0(w5)));
        Round(d, e, f, g, h, a, b, c, K[37] + (w5 += sigma1(w3) + w14 + sigma0(w6)));
        Round(c, d, e, f, g, h, a, b, K[38] + (w6 += sigma1(w4) + w15 + sigma0(w7)));
        Round(b, c, d, e, f, g, h, a, K[39] + (w7 += sigma1(w5) + w0 + sigma0(w8)));
        Round(a, b, c, d, e, f, g, h, K[40] + (w8 += sigma1(w6) + w1 + sigma0(w9)));
        Round(h, a, b, c, d, e, f, g, K[41] + (w9 += sigma1(w7) + w2 + sigma0(w10)));
        Round(g, h, a, b, c, d, e, f, K[42] + (w10 += sigma1(w8) + w3 + sigma0(w11)));
        Round(f, g, h, a, b, c, d, e, K[43] + (w11 += sigma1(w9) + w4 + sigma0(w12)));
        Round(e, f, g, h, a, b, c, d, K[44] + (w12 += sigma1(w10) + w5 + sigma0(w13)));
        Round(d, e, f, g, h, a, b, c, K[45] + (w13 += sigma1(w11) + w6 + sigma0(w14)));
        Round(c, d, e, f, g, h, a, b, K[46] + (w14 += sigma1(w12) + w7 + sigma0(w15)));
        Round(b, c, d, e, f, g, h, a, K[47] + (w15 += sigma1(w13) + w8 + sigma0(w0)));

        Round(a, b, c, d, e, f, g, h, K[48] + (w0 += sigma1(w14) + w9 + sigma0(w1)));
        Round(h, a, b, c, d, e, f, g, K[49] + (w1 += sigma1(w15) + w10 + sigma0(w2)));
        Round(g, h, a, b, c, d, e, f, K[50] + (w2 += sigma1(w0) + w11 + sigma0(w3)));
        Round(f, g, h, a, b, c, d, e, K[51] + (w3 += sigma1(w1) + w12 + sigma0(w4)));
        Round(e, f, g, h, a, b, c, d, K[52] + (w4 += sigma1(w2) + w13 + sigma0(w5)));
        Round(d, e, f, g, h, a, b, c, K[53] + (w5 += sigma1(w3) + w14 + sigma0(w6)));
        Round(c, d, e, f, g, h, a, b, K[54] + (w6 += sigma1(w4) + w15 + sigma0(w7)));
        Round(b, c, d, e, f, g, h, a, K[55] + (w7 += sigma1(w5) + w0 + sigma0(w8)));
        Round(a, b, c, d, e, f, g, h, K[56] + (w8 += sigma1(w6) + w1 + sigma0(w9)));
        Round(h, a, b, c, d, e, f, g, K[57] + (w9 += sigma1(w7) + w2 + sigma0(w10)));
        Round(g, h, a, b, c, d, e, f, K[58] + (w10 += sigma1(w8) + w3 + sigma0(w11)));
        Round(f, g, h, a, b, c, d, e, K[59] + (w11 += sigma1(w9) + w4 + sigma0(w12)));
        Round(e, f, g, h, a, b, c, d, K[60] + (w12 += sigma1(w10) + w5 + sigma0(w13)));
        Round(d, e, f, g, h, a, b, c, K[61] + (w13 += sigma1(w11) + w6 + sigma0(w14)));
        Round(c, d, e, f, g, h, a, b, K[62] + (w14 += sigma1(w12) + w7 + sigma0(w15)));
        Round(b, c, d, e, f, g, h, a, K[63] + (w15 += sigma1(w13) + w8 + sigma0(w0)));

        s[0] += a;
        s[1] += b;
        s[2] += c;
        s[3] += d;
        s[4] += e;
        s[5] += f;
        s[6] += g;
        s[7] += h;
        chunk += 64;
    }
}

void TransformD64_C(unsigned char* out, const unsigned char* in)
{
    uint32_t s[8];
    static const unsigned char padding1[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0
    };
    unsigned char buf[64];

    s[0] = 0x6a09e667ul;
    s[1] = 0xbb67ae85ul;
    s[2] = 0x3c6ef372ul;
    s[3] = 0xa54ff53aul;
    s[4] = 0x510e527ful;
    s[5] = 0x9b05688cul;
    s[6] = 0x1f83d9abul;
    s[7] = 0x5be0cd19ul;
    Transform_C(s, in, 1);
    Transform_C(s, padding1, 1);

    WriteBE32(buf + 0, s[0]);
    WriteBE32(buf + 4, s[1]);
    WriteBE32(buf + 8, s[2]);
    WriteBE32(buf + 12, s[3]);
    WriteBE32(buf + 16, s[4]);
    WriteBE32(buf + 20, s[5]);
    WriteBE32(buf + 24, s[6]);
    WriteBE32(buf + 28, s[7]);
    buf[32] = 0x80;
    memset(buf + 33, 0, 23);
    buf[62] = 0x01;
    buf[63] = 0x00;

    s[0] = 0x6a09e667ul;
    s[1] = 0xbb67ae85ul;
    s[2] = 0x3c6ef372ul;
    s[3] = 0xa54ff53aul;
    s[4] = 0x510e527ful;
    s[5] = 0x9b05688cul;
    s[6] = 0x1f83d9abul;
    s[7] = 0x5be0cd19ul;
    Transform_C(s, buf, 1);

    WriteBE32(out + 0, s[0]);
    WriteBE32(out + 4, s[1]);
    WriteBE32(out + 8, s[2]);
    WriteBE32(out + 12, s[3]);
    WriteBE32(out + 16, s[4]);
    WriteBE32(out + 20, s[5]);
    WriteBE32(out + 24, s[6]);
    WriteBE32(out + 28, s[7]);
}

}

#if defined(__x86_64__) || defined(__amd64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
namespace sha256_x86_shani {
void Transform(uint32_t* s, const unsigned char* chunk, size_t blocks);
void TransformD64(unsigned char* out, const unsigned char* in);
}
namespace sha256d64_sse41 {
void Transform_4way(unsigned char* out, const unsigned char* in);
}
#endif

void (*sha256_transform)(uint32_t*, const unsigned char*, size_t) = Transform_C;
void (*sha256d64_transform)(unsigned char*, const unsigned char*) = TransformD64_C;

static bool g_have_sse4 = false;
static bool g_have_avx2 = false;
static bool g_have_shani = false;
static std::string g_sha256_impl_name = "standard";

std::string SHA256AutoDetect(sha256_implementation::UseImplementation use_implementation)
{
#ifdef HAVE_GETCPUID
    uint32_t eax, ebx, ecx, edx;

    GetCPUID(1, 0, eax, ebx, ecx, edx);
    g_have_sse4 = (ecx >> 19) & 1;

    GetCPUID(7, 0, eax, ebx, ecx, edx);
    g_have_avx2 = (ebx >> 5) & 1;
    g_have_shani = (ebx >> 29) & 1;

#if defined(ENABLE_X86_SHANI)
    if (g_have_shani && (use_implementation & sha256_implementation::USE_SHANI)) {
        sha256_transform = sha256_x86_shani::Transform;
        sha256d64_transform = sha256_x86_shani::TransformD64;  // Also set the D64 version!
        g_sha256_impl_name = "shani(1way)";
        printf("SHA256: Using SHA-NI hardware acceleration\n");
        return g_sha256_impl_name;
    }
#endif

#if defined(ENABLE_SSE41)
    if (g_have_sse4 && (use_implementation & sha256_implementation::USE_SSE4)) {
        g_sha256_impl_name = "sse4(1way)";
        printf("SHA256: Using SSE4.1 acceleration\n");
        return g_sha256_impl_name;
    }
#endif

    if (g_have_avx2) {
        printf("SHA256: AVX2 detected but using standard implementation\n");
    }
#endif

    g_sha256_impl_name = "standard";
    printf("SHA256: Using standard C implementation\n");
    return g_sha256_impl_name;
}

CSHA256::CSHA256()
{
    Reset();
}

CSHA256& CSHA256::Write(const unsigned char* data, size_t len)
{
    const unsigned char* end = data + len;
    size_t bufsize = bytes % 64;
    if (bufsize && bufsize + len >= 64) {
        memcpy(buf + bufsize, data, 64 - bufsize);
        bytes += 64 - bufsize;
        data += 64 - bufsize;
        sha256_transform(s, buf, 1);
        bufsize = 0;
    }
    if (end - data >= 64) {
        size_t blocks = (end - data) / 64;
        sha256_transform(s, data, blocks);
        data += 64 * blocks;
        bytes += 64 * blocks;
    }
    if (end > data) {
        memcpy(buf + bufsize, data, end - data);
        bytes += end - data;
    }
    return *this;
}

void CSHA256::Finalize(unsigned char hash[OUTPUT_SIZE])
{
    static const unsigned char pad[64] = {0x80};
    unsigned char sizedesc[8];
    uint64_t bits = bytes << 3;
    sizedesc[0] = bits >> 56;
    sizedesc[1] = bits >> 48;
    sizedesc[2] = bits >> 40;
    sizedesc[3] = bits >> 32;
    sizedesc[4] = bits >> 24;
    sizedesc[5] = bits >> 16;
    sizedesc[6] = bits >> 8;
    sizedesc[7] = bits;
    Write(pad, 1 + ((119 - (bytes % 64)) % 64));
    Write(sizedesc, 8);
    WriteBE32(hash, s[0]);
    WriteBE32(hash + 4, s[1]);
    WriteBE32(hash + 8, s[2]);
    WriteBE32(hash + 12, s[3]);
    WriteBE32(hash + 16, s[4]);
    WriteBE32(hash + 20, s[5]);
    WriteBE32(hash + 24, s[6]);
    WriteBE32(hash + 28, s[7]);
}

CSHA256& CSHA256::Reset()
{
    bytes = 0;
    s[0] = 0x6a09e667ul;
    s[1] = 0xbb67ae85ul;
    s[2] = 0x3c6ef372ul;
    s[3] = 0xa54ff53aul;
    s[4] = 0x510e527ful;
    s[5] = 0x9b05688cul;
    s[6] = 0x1f83d9abul;
    s[7] = 0x5be0cd19ul;
    return *this;
}

void SHA256D64(unsigned char* output, const unsigned char* input, size_t blocks)
{
    while (blocks--) {
        sha256d64_transform(output, input);
        output += 32;
        input += 64;
    }
}

void SHA256Transform(uint32_t* state, const unsigned char* data)
{
    sha256_transform(state, data, 1);
}

namespace sha256 {
void Transform(uint32_t* s, const unsigned char* chunk, size_t blocks)
{
    sha256_transform(s, chunk, blocks);
}

void TransformD64(unsigned char* out, const unsigned char* in)
{
    sha256d64_transform(out, in);
}
}

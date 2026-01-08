// Copyright (c) 2014-present The Bitcoin Core developers
// Adapted for Bitok - optimized SHA256 implementation
// Distributed under the MIT software license

#ifndef BITOK_CRYPTO_SHA256_H
#define BITOK_CRYPTO_SHA256_H

#include <cstdint>
#include <cstdlib>
#include <string>

class CSHA256
{
private:
    uint32_t s[8];
    unsigned char buf[64];
    uint64_t bytes{0};

public:
    static const size_t OUTPUT_SIZE = 32;

    CSHA256();
    CSHA256& Write(const unsigned char* data, size_t len);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    CSHA256& Reset();
};

namespace sha256_implementation {
enum UseImplementation : uint8_t {
    STANDARD = 0,
    USE_SSE4 = 1 << 0,
    USE_AVX2 = 1 << 1,
    USE_SHANI = 1 << 2,
    USE_SSE4_AND_AVX2 = USE_SSE4 | USE_AVX2,
    USE_SSE4_AND_SHANI = USE_SSE4 | USE_SHANI,
    USE_ALL = USE_SSE4 | USE_AVX2 | USE_SHANI,
};
}

std::string SHA256AutoDetect(sha256_implementation::UseImplementation use_implementation = sha256_implementation::USE_ALL);

void SHA256D64(unsigned char* output, const unsigned char* input, size_t blocks);

void SHA256Transform(uint32_t* state, const unsigned char* data);

namespace sha256 {
    void Transform(uint32_t* s, const unsigned char* chunk, size_t blocks);
    void TransformD64(unsigned char* out, const unsigned char* in);
}

#endif // BITOK_CRYPTO_SHA256_H

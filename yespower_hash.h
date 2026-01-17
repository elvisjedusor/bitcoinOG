/*
 * Yespower hash integration for Bitcoin
 * CPU-friendly, GPU/ASIC-resistant proof-of-work
 */

#ifndef YESPOWER_HASH_H
#define YESPOWER_HASH_H

#include <stdint.h>
#include <string.h>
#include "uint256.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "yespower.h"

#ifdef __cplusplus
}
#endif

#define YESPOWER_N 2048
#define YESPOWER_R 32

static const char* YESPOWER_PERS = "BitokPoW";
static const size_t YESPOWER_PERSLEN = 8;

static inline const yespower_params_t* get_yespower_params() {
    static yespower_params_t params = {YESPOWER_1_0, YESPOWER_N, YESPOWER_R,
                                        (const uint8_t*)YESPOWER_PERS, YESPOWER_PERSLEN};
    return &params;
}

inline void yespower_hash(const char* input, size_t inputlen, char* output)
{
    yespower_binary_t dst;
    if (yespower_tls((const uint8_t*)input, inputlen, get_yespower_params(), &dst) == 0) {
        memcpy(output, dst.uc, 32);
    } else {
        memset(output, 0xff, 32);
    }
}

inline uint256 YespowerHash(const void* pbegin, const void* pend)
{
    uint256 result;
    size_t len = (const char*)pend - (const char*)pbegin;
    yespower_hash((const char*)pbegin, len, (char*)&result);
    return result;
}

inline uint256 YespowerHashWithLocal(yespower_local_t* local, const void* pbegin, const void* pend)
{
    uint256 result;
    size_t len = (const char*)pend - (const char*)pbegin;
    yespower_binary_t dst;
    if (yespower(local, (const uint8_t*)pbegin, len, get_yespower_params(), &dst) == 0) {
        memcpy(&result, dst.uc, 32);
    } else {
        memset(&result, 0xff, 32);
    }
    return result;
}

inline uint256 YespowerHashBlock(const void* pblock, size_t len)
{
    uint256 result;
    yespower_hash((const char*)pblock, len, (char*)&result);
    return result;
}

#endif /* YESPOWER_HASH_H */

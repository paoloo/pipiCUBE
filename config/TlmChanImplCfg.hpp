// Reduced TlmChan hash-table sizes for RP2040 (264 KB RAM).
// Default TLMCHAN_HASH_BUCKETS=500 produces ~528 KB BSS (double-buffered
// 500 * sizeof(TlmEntry) * 2); 25 buckets fit our ~20 actual channels.
#ifndef TLMCHANIMPLCFG_HPP_
#define TLMCHANIMPLCFG_HPP_

namespace {
enum {
    TLMCHAN_NUM_TLM_HASH_SLOTS = 5,
    TLMCHAN_HASH_MOD_VALUE     = 23,
    TLMCHAN_HASH_BUCKETS       = 25
};
}

#endif

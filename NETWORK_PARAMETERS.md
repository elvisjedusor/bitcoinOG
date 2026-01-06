# Network Parameter Comparison: Satoshi's Bitcoin vs BitcoinOG

This document provides a comprehensive comparison of all network parameters between Satoshi Nakamoto's original Bitcoin implementation and the modified BitcoinOG version.

## Genesis Block Parameters

| Parameter | Original Bitcoin (Satoshi) | BitcoinOG | Change |
|-----------|---------------------------|-----------|--------|
| **Genesis Hash** | `0x000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f` | `0x00000ac60cbeeac0c7cf58c88495e9db3ea63dae1c06c01eaedcee2ba296dbab` | New genesis mined |
| **Genesis Timestamp** | `1231006505` (Jan 3, 2009) | `1231006505` (same) | No change |
| **Genesis Message** | "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks" | Same | No change |
| **Genesis nBits** | `0x1d00ffff` | `0x1e0ffff0` | Easier difficulty |
| **Genesis nNonce** | 2083236893 | Remined with new difficulty | Different nonce |
| **Genesis Public Key** | Original Satoshi key | Different key | New key generated |

## Proof of Work / Difficulty

| Parameter | Original Bitcoin | BitcoinOG | Ratio |
|-----------|-----------------|-----------|-------|
| **PoW Limit** | `~uint256(0) >> 32` | `~uint256(0) >> 20` | **4096x easier** |
| **Initial Difficulty** | ~32 bits of zeros required | ~20 bits of zeros required | Much easier for CPU |
| **Target Use Case** | ASIC/GPU mining (2009: CPU) | VPS/CPU-friendly mining | Optimized for modern CPUs |

The PoW limit change is the most critical modification, making mining **4096 times easier** than original Bitcoin. This allows mining on basic CPUs and VPS instances without specialized hardware.

## Block Timing & Difficulty Retargeting

| Parameter | Original Bitcoin | BitcoinOG | Ratio |
|-----------|-----------------|-----------|-------|
| **Target Block Time** | 10 minutes | **1 minute** | **10x faster** |
| **Retarget Interval** | 2016 blocks | **10 blocks** | **201.6x more frequent** |
| **Retarget Timespan** | 14 days (2016 × 10 min) | **10 minutes** (10 × 1 min) | **2016x faster** |
| **Difficulty Adjustment Limit** | 4x up/down per retarget | 4x up/down per retarget | Same safety mechanism |
| **Time Between Retargets** | ~2 weeks | ~10 minutes | Much more responsive |

### Retargeting Formula (Unchanged)

Both versions use the same difficulty adjustment formula:
```
new_difficulty = old_difficulty * (actual_timespan / target_timespan)
```

With bounds:
```
actual_timespan = max(target_timespan/4, min(actual_timespan, target_timespan*4))
```

## Block Rewards & Coin Supply

| Parameter | Original Bitcoin | BitcoinOG | Ratio |
|-----------|-----------------|-----------|-------|
| **Initial Block Reward** | 50 BTC | **5 BTCOG** | **1/10th** |
| **Halving Interval** | 210,000 blocks | **2,100,000 blocks** | **10x longer in blocks** |
| **Time to First Halving** | ~4 years (210k × 10 min) | **~4 years** (2.1M × 1 min) | **Same real-world time** |
| **Maximum Supply** | ~21,000,000 BTC | **~21,000,000 BTCOG** | Same total cap |
| **Coinbase Maturity** | 100 blocks | **10 blocks** | **1/10th in blocks** |
| **Maturity Time** | ~16.67 hours (100 × 10 min) | **~10 minutes** (10 × 1 min) | Much faster |

## Transaction Confirmation Requirements

### Coinbase (Mining Rewards) Transactions

| Parameter | Original Bitcoin | BitcoinOG | Notes |
|-----------|-----------------|-----------|-------|
| **Minimum to Display in UI** | 2 confirmations | 2 confirmations | Before showing in wallet |
| **Blocks Until Spendable** | 120 blocks (100 + 20) | **30 blocks (10 + 20)** | `GetBlocksToMaturity()` |
| **Real-World Wait Time** | ~20 hours (120 × 10 min) | **~30 minutes** (30 × 1 min) | Much faster |
| **Formula** | `(COINBASE_MATURITY + 20) - depth` | Same formula | Extra 20 blocks safety buffer |

**Why the +20 buffer?**
- Coinbase transactions have an extra 20-block safety margin beyond the maturity period
- This is implemented in `GetBlocksToMaturity()` in main.cpp:629
- Protects against chain reorganizations that could invalidate mined blocks

### Standard (User-to-User) Transactions

| Parameter | Original Bitcoin | BitcoinOG | Notes |
|-----------|-----------------|-----------|-------|
| **Minimum Confirmations** | 1 confirmation | 1 confirmation | Default in RPC (rpc.cpp:354) |
| **Display Immediately** | Yes, once in block | Yes, once in block | Shows with 0 confirmations |
| **Considered "Confirmed"** | 6 confirmations | 6 confirmations | Industry standard |
| **Real-World for 6 Confirmations** | ~60 minutes | **~6 minutes** | **10x faster** |

**Key Differences:**
- Standard transactions appear in wallet immediately (depth > 0)
- Coinbase transactions hidden until 2+ confirmations
- Standard transactions spendable after 1 confirmation
- Coinbase transactions require full maturity period (30 blocks for BitcoinOG)

### Reward Schedule Comparison

| Epoch | Original Bitcoin | BitcoinOG | Real Time |
|-------|-----------------|-----------|-----------|
| **0** | 50 BTC | 5 BTCOG | Years 0-4 |
| **1** | 25 BTC | 2.5 BTCOG | Years 4-8 |
| **2** | 12.5 BTC | 1.25 BTCOG | Years 8-12 |
| **3** | 6.25 BTC | 0.625 BTCOG | Years 12-16 |
| **...** | ... | ... | ... |
| **∞** | ~21M total | ~21M total | ~140 years |

### Block Reward Code

**Original Bitcoin:**
```cpp
int64 CBlock::GetBlockValue(int64 nFees) const
{
    int64 nSubsidy = 50 * COIN;
    nSubsidy >>= (nBestHeight / 210000);  // Halving
    return nSubsidy + nFees;
}
```

**BitcoinOG:**
```cpp
int64 CBlock::GetBlockValue(int64 nFees) const
{
    // BitcoinOG: 5 coins per block (1/10th of Bitcoin)
    int64 nSubsidy = 5 * COIN;

    // BitcoinOG: Halving every 2,100,000 blocks (proportionally scaled)
    nSubsidy >>= (nBestHeight / 2100000);

    return nSubsidy + nFees;
}
```

## Block Size & Transaction Limits

| Parameter | Original Bitcoin | BitcoinOG | Change |
|-----------|-----------------|-----------|--------|
| **Max Block Size** | `0x02000000` (32 MB) | `0x02000000` (32 MB) | Same |
| **Coin Unit** | 100,000,000 satoshis | 100,000,000 satoshis | Same precision |
| **Cent Unit** | 1,000,000 satoshis | 1,000,000 satoshis | Same |

## Transaction Fee Rules

Both versions use identical transaction fee rules:

- Base fee: 1 cent per kilobyte
- Transactions under 60K: free if block size < 80K
- Transactions under 3K: free if block size < 200K
- Dust spam protection: 0.01 fee if any output < 0.01

## Network Identity Parameters

### Changed Parameters (Network Isolation)

These parameters were changed to create a separate network and prevent cross-contamination with Bitcoin:

| Parameter | Original Bitcoin | BitcoinOG | Purpose |
|-----------|-----------------|-----------|---------|
| **Message Start Magic** | `0xf9, 0xbe, 0xb4, 0xd9` | `0xb4, 0x0b, 0xc0, 0xde` | Network message identification |
| **Default P2P Port** | 8333 (0x8d20) | **18333** | Peer-to-peer communication |
| **IRC Channel** | `#bitcoin` | `#bitcoinog` | Peer discovery channel |
| **IRC Server** | irc.lfnet.org:6667 | irc.lfnet.org:6667 | Same server, different channel |

**Why These Changes Matter:**

1. **pchMessageStart** - The 4-byte network magic prevents nodes from different networks from communicating. Each message starts with these bytes, so Bitcoin and BitcoinOG nodes will reject each other's messages.

2. **DEFAULT_PORT** - Using a different port (18333 vs 8333) allows both Bitcoin and BitcoinOG to run on the same machine without port conflicts.

3. **IRC Channel** - Separate IRC channel ensures peer discovery only finds BitcoinOG nodes, not Bitcoin nodes.

These changes completely isolate BitcoinOG from the Bitcoin network while using the same protocols and data structures.

### Code References

- **net.h:15** - `DEFAULT_PORT = htons(18333)`
- **net.h:55** - `pchMessageStart[4] = { 0xb4, 0x0b, 0xc0, 0xde }`
- **irc.cpp:231** - `JOIN #bitcoinog`

## Network Constants (Unchanged)

These parameters remain identical:

| Parameter | Value |
|-----------|-------|
| **Protocol Version** | 300 |
| **Protocol Sub-Version** | "" (empty) |
| **Address Version** | 0 |
| **Serialization Format** | Same |
| **Address Format** | Same (Base58Check) |
| **Script System** | Same |
| **Signature Algorithm** | ECDSA (same curve) |
| **Hash Functions** | SHA-256, RIPEMD-160 (same) |

## Key Design Rationale

### Why These Specific Changes?

1. **Difficulty (4096x easier)**
   - Original Bitcoin: Designed for 2009-era CPUs, now requires ASICs
   - BitcoinOG: Targets modern VPS/CPU mining without specialized hardware
   - Maintains security through network participation

2. **Block Time (10 min → 1 min)**
   - Faster transaction confirmations
   - Better user experience for modern applications
   - 10x more blocks to maintain economic model

3. **Rewards (50 → 5 coins)**
   - Compensates for 10x more blocks
   - Maintains same inflation rate
   - Same total supply (~21M)

4. **Halving (210k → 2.1M blocks)**
   - 10x more blocks needs 10x longer interval
   - Preserves ~4 year halving cycle
   - Same long-term economics

5. **Retargeting (2016 → 10 blocks)**
   - Faster difficulty adjustments
   - More responsive to hashrate changes
   - Maintains same 4x adjustment limits

6. **Maturity (100 → 10 blocks)**
   - Same real-world time (~10 minutes)
   - Proportional to faster block time
   - Prevents premature coinbase spending

### Economic Equivalence

Despite parameter changes, BitcoinOG maintains Bitcoin's core economic properties:

| Economic Property | Status |
|------------------|--------|
| **Max Supply** | ✓ Same (~21M) |
| **Halving Cycle** | ✓ Same (~4 years) |
| **Inflation Rate** | ✓ Same over time |
| **Supply Curve** | ✓ Identical shape |
| **Scarcity Model** | ✓ Preserved |

### Real-World Time Comparison

| Event | Original Bitcoin | BitcoinOG | Difference |
|-------|-----------------|-----------|------------|
| **Block confirmation** | 10 minutes | 1 minute | 10x faster |
| **Standard tx (1 conf)** | 10 minutes | 1 minute | 10x faster |
| **Standard tx (6 conf)** | 1 hour | 6 minutes | 10x faster |
| **Coinbase display (2 conf)** | ~20 minutes | ~2 minutes | 10x faster |
| **Coinbase spendable (120/30)** | ~20 hours | ~30 minutes | 40x faster |
| **Difficulty retarget** | 2 weeks | 10 minutes | 2016x faster |
| **First halving** | ~4 years | ~4 years | Same |
| **Full emission** | ~140 years | ~140 years | Same |

## Source Code References

### Main Files Modified

- **main.h:20** - `COINBASE_MATURITY = 10` (was 100)
- **main.h:23** - `bnProofOfWorkLimit(~uint256(0) >> 20)` (was >> 32)
- **main.cpp:25** - Genesis hash updated
- **main.cpp:797-806** - `GetBlockValue()` function
- **main.cpp:808-853** - `GetNextWorkRequired()` function
- **main.cpp:1545-1618** - Genesis block creation and mining

## Summary

BitcoinOG implements a **proportionally scaled** version of Bitcoin that:

- ✓ Maintains identical economic model (21M supply, 4-year halvings)
- ✓ Enables CPU mining on modern hardware (4096x easier PoW)
- ✓ Provides faster confirmations (1-minute blocks)
- ✓ Preserves Bitcoin's security and incentive mechanisms
- ✓ Keeps the same transaction and scripting system

All changes are mathematically consistent to preserve Bitcoin's fundamental properties while adapting for CPU mining and faster transaction processing.

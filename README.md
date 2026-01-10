# Bitok 0.3.19 (Mainnet is Live)

Bitcoin was never meant to be finished. It was meant to be unleashed. Satoshi Nakomoto wrote the code, launched the network, fixed critical bugs, and then walked away. That was not abandonment. That was completion. A system that depends on its creator is not decentralized. v0.3.19 was the last release under Satoshi's direct involvement. Everything after that is history. Not destiny.

Bitok is Bitcoin v0.3.19 - the complete Satoshi-era codebase, run as a separate chain from genesis. The same rules. The same behavior. The same philosophy. All critical security fixes in place from day one. Adapted only as much as required **to run** on modern operating systems and **resist GPU mining**. No features added. No ideology injected. No attempt to "fix" Bitcoin according to modern tastes.

This is not BTC. It does not compete with BTC. BTC is what Bitcoin became after years of social negotiation and ideological drift. Bitok is what Bitcoin was before it was captured by its own caretakers.

## Overview

Bitok implements the complete Satoshi-era codebase (v0.3.19) with these intentional modifications:

- Modern build compatibility on contemporary toolchains

- CPU-friendly proof-of-work via Yespower (memory-hard algorithm replacing SHA-256)

- Independent mainnet with hardcoded parameters (genesis, peers, IRC bootstrapping)

- Command-line configuration only (no config file)

## Mainnet is live

The genesis block and network parameters are hardcoded in source (main.cpp).

### Genesis Block

```
Hash:       0x0290400ea28d3fe79d102ca6b7cd11cee5eba9f17f2046c303d92f65d6ed2617
Timestamp:  "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks"
Unix Time:  1231006505 (January 3, 2009)
nVersion:   1
nBits:      0x1effffff
nNonce:     37137
Coinbase:   50 BITOK
```
// The genesis block retains the original Bitcoin timestamp (January 3, 2009) as a tribute to Satoshi's creation. This has no practical impact on the network since difficulty adjustment is based on block count, not calendar time. The historical timestamp only affects the first retarget calculation.

### Network Parameters

#### Blockchain Constants

| Parameter | Value | Description |
|-----------|-------|-------------|
| **PoW Algorithm** | Yespower 1.0 | ASIC-resistant, CPU-optimized (N=2048, r=32) pers="BitokPoW" |
| **Block Time** | 10 minutes | Target time between blocks (600 seconds) |
| **Block Reward** | 50 BITOK | Initial coinbase reward |
| **Halving Interval** | 210,000 blocks | Reward halves approximately every 4 years |
| **Max Supply** | 21,000,000 BITOK | Hard cap on total coins |
| **Smallest Unit** | 0.00000001 BITOK | 1 satoshi = 10^-8 BITOK |
| **Max Block Size** | 1 MB | Maximum block payload (1,000,000 bytes) |
| **Coinbase Maturity** | 100 blocks | Confirmations before mining reward spendable |
| **Difficulty Adjustment** | 2016 blocks | Retarget interval (~2 weeks) |
| **Difficulty Window** | 14 days | Target timespan (1,209,600 seconds) |
| **Difficulty Clamp** | 4x | Max adjustment per retarget (0.25x to 4x) |
| **Initial Difficulty** | `0x1effffff` | bnProofOfWorkLimit = ~uint256(0) >> 17 |

#### Network Ports

| Parameter | Value |
|-----------|-------|
| **P2P Port** | 18333 |
| **RPC Port** | 8332 |
| **Message Start** | 0xb40bc0de |

#### Fee Structure

| Fee Type | Amount | Description |
|----------|--------|-------------|
| **Base Fee Rate** | 0.01 BITOK/KB | Per transaction, based on tx size (rounded up) |
| **Free Tier 1** | 0 BITOK | Tx < 3 KB when block < 200 KB |
| **Free Tier 2** | 0 BITOK | Tx < 60 KB when block < 80 KB |
| **Dust Penalty** | 0.01 BITOK minimum | If any output < 0.01 BITOK |

### IRC Bootstrap

Peer discovery uses IRC bootstrap, exactly as early Bitcoin releases did. Parameters are hardcoded and may change availability over time; alternative peers must be discovered organically.

### Security Warning

**If you lose your wallet.dat file or forget your passphrase, your coins are permanently lost.** There is no recovery mechanism, no password reset, no customer support. You are responsible for your keys. You are responsible for your backups. You are responsible for your mistakes.

The protocol logic, networking, transaction validation, and wallet behavior implement Bitcoin v0.3.19 - Satoshi's final release with all critical security fixes in place. This repository reflects the exact mainnet now running. The protocol is complete. No further consensus changes will be made.

---

### Technical Documentation
- [RPC API Reference](RPC_API.md) - complete JSON-RPC API for exchanges, mining pools, block explorers
- [Yespower Mining Algorithm](BITOKPOW.md) - technical details on CPU-optimized proof-of-work
- [Project Manifesto](MANIFESTO.md) - philosophy and purpose

### Build Guides
- [Unix/Linux Build Guide](BUILD_UNIX.md)
- [macOS Build Guide](BUILD_MACOS.md)
- [Windows Build Guide](BUILD_WINDOWS.md)

### Legal & Resources
- [License](license.txt) - MIT/X11 License
- [Links](#links)

---

## What Changed From Original Bitcoin

### Code Changes Summary

| Category | Changes | Purpose |
|----------|---------|---------|
| System Compatibility | OpenSSL 3.x, Berkeley DB (modern compatible version), Boost 1.74+, GCC 11+, wxWidgets 3.2 | compile on modern Ubuntu 24.04 |
| Proof-of-Work | SHA-256 → Yespower 1.0 (N=2048, r=32) | CPU-friendly, GPU/ASIC-resistant |
| Network | new genesis block | separate network from BTC |

### Security Fixes Inherited from v0.3.19

Between Bitcoin v0.3.0 (July 2010) and v0.3.19 (December 2010), Satoshi addressed critical security issues:

- **Value Overflow Protection** (v0.3.9, August 2010) - Fixed integer overflow bug that allowed creating 184 billion coins. Bitcoin forked to reject the invalid chain. Bitok launches with this fix already in place.
- **Blockchain Checkpoints** (v0.3.2) - Prevents deep chain reorganizations
- **DoS Protection** (v0.3.19) - Block size limits, message limits, resource exhaustion protection
- **IsStandard() Filter** (v0.3.18) - Only relay and mine known transaction types

Unlike Bitcoin, which forked once to fix the overflow bug, Bitok launches with all security fixes from day one. No forks. No compromises. The protocol is complete.

No features. No layers. No "improvements."

See [BITOKPOW.md](BITOKPOW.md) for technical details on the Yespower proof-of-work algorithm.

## Building

### Dependencies (Ubuntu 24.04)

```bash
sudo apt-get update
sudo apt-get install build-essential libssl-dev libdb-dev libdb5.3-dev libboost-all-dev

# For GUI (optional)
sudo apt-get install libwxgtk3.2-dev libgtk-3-dev
```

### Compile

```bash
# Daemon only
make -f makefile.unix

# GUI wallet
make -f makefile.unix gui

# Both
make -f makefile.unix all
```

See [BUILD_UNIX.md](BUILD_UNIX.md) for detailed build instructions for all platforms.

## Running

### Daemon

```bash
./bitokd                    # start node
./bitokd -gen               # start node with mining enabled
./bitokd -daemon -gen       # run in background with mining
./bitokd getinfo            # get node info
./bitokd help               # list all commands
```

### GUI

```bash
./bitok                     # launch graphical wallet
```

The GUI provides a user-friendly interface for:
- sending and receiving coins
- viewing transaction history
- generating new addresses
- controlling mining

## Mining

Mining on Bitok works exactly as it did in early Bitcoin (v0.3.19), except it uses Yespower instead of SHA-256.

Enable mining:
```bash
./bitokd -gen                           # start mining on all CPU cores
./bitokd -gen -genproclimit=4           # limit to 4 cores
```

In the GUI: Settings → Options → Generate Coins

Why Yespower:
- memory-hard algorithm (requires ~128KB RAM per hash)
- CPU-optimized with automatic SIMD detection (SSE2/AVX/AVX2/AVX512)
- GPU/ASIC-resistant by design
- maintains Satoshi's vision: anyone with a laptop can mine

See [BITOKPOW.md](BITOKPOW.md) for technical details on the mining algorithm.

## Philosophy

Read [MANIFESTO.md](MANIFESTO.md) for the full context.

Short version:

Bitcoin was meant to be peer-to-peer electronic cash. Mining was meant to be accessible to everyone. The protocol was meant to be simple and unchanging. Privacy was meant to be natural, not bolted on. There were no leaders, no roadmaps, no foundations.

Bitcoin v0.3.19 represented the completion of Satoshi's design - all critical security issues addressed, protocol stable and tested. Bitok preserves that complete design.

## What This Is Not

This is not BTC. It is Different network, different genesis, different mining algorithm.

This is not a proposal. No one needs to agree. Run it or don't.

This is not a political fork. No debate, no governance, no ideology injection.

This is not trying to be Bitcoin. Bitcoin already exists and has made its choices.


**This is software that runs. If you run it, you are the network. If you don't, you aren't.**


## Security Notice

This is code from 2010, adapted for modern systems. The chain implements Satoshi-era consensus design; modern additions such as BIP protocols and script upgrades are intentionally absent.

The same security model as Bitcoin v0.3.19 - all critical fixes in place, protocol complete and stable. Modern cryptography (OpenSSL 3.x). GPU-resistant mining (Yespower). No post-Satoshi "features" (good). No guarantees (as intended).

You are responsible for your keys. You are responsible for your node. You are responsible for your mistakes.

## Documentation

### For Developers & Integrators
- [RPC_API.md](RPC_API.md) - complete JSON-RPC API reference for exchanges, mining pools, and block explorers

### Philosophy & Technical Details
- [MANIFESTO.md](MANIFESTO.md) - philosophy and purpose
- [BITOKPOW.md](BITOKPOW.md) - technical details on Yespower mining algorithm

### Build Instructions
- [BUILD_UNIX.md](BUILD_UNIX.md) - Unix/Linux build instructions
- [BUILD_MACOS.md](BUILD_MACOS.md) - macOS build instructions
- [BUILD_WINDOWS.md](BUILD_WINDOWS.md) - Windows build instructions

## License

MIT/X11 License - See [license.txt](license.txt)

## Authors

Satoshi Nakamoto (2009-2010)
Tom Elvis Jedusor - system compatibility updates and Yespower integration (present)

## Links

Repository: https://github.com/elvisjedusor/bitok

Original Bitcoin: https://bitcoin.org/bitcoin.pdf

Yespower: https://www.openwall.com/yespower/

---

Run the code. That is the manifesto.

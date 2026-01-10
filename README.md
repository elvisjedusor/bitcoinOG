# Bitok 0.3.0 (Mainnet)

Bitok 0.3.0 is a restored release of Bitcoin v0.3.0 (2010), the last version of Bitcoin personally released and configured by Satoshi Nakamoto. The software preserves the original behavior and operating model of early Bitcoin, while adapting mining to keep GPU and ASIC hardware inefficient.

BITOK is not BTC.
It does not connect to, interact with, or fork the Bitcoin mainnet.

## Overview

Bitok preserves the original Bitcoin v0.3.0 codebase with the following intentional modifications:

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

### Network Parameters

```
P2P Port:       18333
RPC Port:       8332
Message Start:  0xb40bc0de
```

### IRC Bootstrap

Peer discovery uses IRC bootstrap, exactly as early Bitcoin releases did. Parameters are hardcoded and may change availability over time; alternative peers must be discovered organically.

### Security Warning

**If you lose your wallet.dat file or forget your passphrase, your coins are permanently lost.** There is no recovery mechanism, no password reset, no customer support. You are responsible for your keys. You are responsible for your backups. You are responsible for your mistakes.

The main protocol logic, networking, transaction validation, and wallet behavior remain consistent with Bitcoin v0.3.0. This repository reflects the exact mainnet now running. No further consensus changes should be expected post-release.

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

## What Changed From Bitcoin v0.3.0

### Code Changes Summary

| Category | Changes | Purpose |
|----------|---------|---------|
| System Compatibility | OpenSSL 3.x, Berkeley DB (modern compatible version), Boost 1.74+, GCC 11+, wxWidgets 3.2 | compile on modern Ubuntu 24.04 |
| Proof-of-Work | SHA-256 → Yespower 1.0 (N=2048, r=32) | CPU-friendly, GPU/ASIC-resistant |
| Network | new genesis block | separate network from BTC |

No features. No protocol changes. No layers. No "improvements."

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

Mining on Bitok works exactly as it did in Bitcoin v0.3.0, except it uses Yespower instead of SHA-256.

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

Bitcoin v0.3.0 embodied these principles. Bitok preserves that design unchanged.

## What This Is Not

This is not BTC. It is Different network, different genesis, different mining algorithm.

This is not a proposal. No one needs to agree. Run it or don't.

This is not a political fork. No debate, no governance, no ideology injection.

This is not trying to be Bitcoin. Bitcoin already exists and has made its choices.


**This is software that runs. If you run it, you are the network. If you don't, you aren't.**


## Security Notice

This is code from 2010, adapted for modern systems. The chain is subject to legacy consensus design; modern defenses such as BIP protocols and script upgrades are absent.

The same security model as Bitcoin v0.3.0. Modern cryptography (OpenSSL 3.x). GPU-resistant mining (Yespower). No modern "features" (good). No guarantees (as intended).

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

Satoshi Nakamoto - original Bitcoin v0.3.0 (2009-2010)
Tom Elvis Jedusor - system compatibility updates and Yespower integration (present)

## Links

Repository: https://github.com/elvisjedusor/bitok

Original Bitcoin: https://bitcoin.org/bitcoin.pdf

Yespower: https://www.openwall.com/yespower/

---

Run the code. That is the manifesto.

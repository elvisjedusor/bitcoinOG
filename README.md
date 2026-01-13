# Bitok

It's Bitcoin, but not Bitcoin. It's new, but also old. And yes, you can actually mine it with your laptop.

## What Is This

I took the last version of Bitcoin that Satoshi worked on (0.3.19 from December 2010), updated it to compile on modern systems, and replaced SHA-256 mining with Yespower so GPUs don't have a massive advantage.

New genesis block. Separate network. Same rules otherwise.

## Why

In 2010, Satoshi wrote:

> It's nice how anyone with just a CPU can compete fairly equally right now.

That stopped being true about a year later. Now it's true again, at least for this chain.

## Quick Start

```bash
# Download and run (Linux)
./bitokd                    # start a node
./bitokd -gen               # start mining
./bitokd getinfo            # check status
```

Mining uses all CPU cores by default. Limit it with `-genproclimit=4` or whatever.

In the GUI: Settings > Options > Generate Coins

## Specifications

| Parameter | Value |
|-----------|-------|
| Algorithm | Yespower 1.0 (CPU-friendly, memory-hard) |
| Block time | 10 minutes |
| Block reward | 50 BITOK, halving every 210,000 blocks |
| Max supply | 21,000,000 |
| P2P port | 18333 |
| RPC port | 8332 |

Same economics as Bitcoin. Different mining algorithm. Different genesis.

## Genesis Block

```
Hash:    0x0290400ea28d3fe79d102ca6b7cd11cee5eba9f17f2046c303d92f65d6ed2617
Message: "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks"
nBits:   0x1effffff
nNonce:  37137
```

The message is the same as Bitcoin's. Seemed appropriate.

## What Changed From Original Bitcoin

**Three things:**

1. Build system updated for OpenSSL 3.x, Boost 1.74+, wxWidgets 3.2, etc. Modern Ubuntu compiles it now.

2. SHA-256 replaced with Yespower for proof-of-work. Your laptop can find blocks. A GPU won't help much.

3. New genesis block. Separate network.

**Everything else is identical to v0.3.19:**

- Same transaction format
- Same script system
- Same wallet behavior
- Same networking code
- Same 21M cap, same halving schedule
- All security fixes from the Satoshi era included

No SegWit. No new opcodes. No BIPs. No layer 2. The protocol is frozen at December 2010.

## Building

### Ubuntu 24.04

```bash
sudo apt-get install build-essential libssl-dev libdb5.3-dev libboost-all-dev

# Daemon only
make -f makefile.unix

# With GUI
sudo apt-get install libwxgtk3.2-dev libgtk-3-dev
make -f makefile.unix gui
```

### Other Platforms

- [BUILD_UNIX.md](BUILD_UNIX.md)
- [BUILD_MACOS.md](BUILD_MACOS.md)
- [BUILD_WINDOWS.md](BUILD_WINDOWS.md)

## Running

**Daemon:**
```bash
./bitokd                          # run node
./bitokd -gen                     # run node + mine
./bitokd -daemon                  # background mode
./bitokd stop                     # stop daemon
```

**Note:** This is original Bitcoin v0.3.19 behavior - there is no config file. All settings must be passed via command line:
```bash
./bitokd -gen -addnode=1.2.3.4 -rpcuser=user -rpcpassword=pass
```

**RPC:**
```bash
./bitokd getinfo                  # node status
./bitokd getbalance               # wallet balance
./bitokd getnewaddress            # new receiving address
./bitokd sendtoaddress <addr> <amount>
./bitokd help                     # list all commands
```

See [RPC_API.md](RPC_API.md) for the full API.

**GUI:**
```bash
./bitok
```

Point and click. Mining checkbox in options.

## Mining

```bash
./bitokd -gen                     # all cores
./bitokd -gen -genproclimit=4     # 4 cores
```

The algorithm uses ~128KB of memory per hash. This is intentional. It's what makes GPUs inefficient.

Your CPU will automatically use SSE2/AVX/AVX2 if available. No configuration needed.

See [BITOKPOW.md](BITOKPOW.md) for technical details on Yespower.

## Data Directory

| OS | Path |
|----|------|
| Linux | `~/.bitok/` |
| macOS | `~/Library/Application Support/Bitok/` |
| Windows | `%APPDATA%\Bitok\` |

Back up `wallet.dat`. If you lose it, coins are gone. There's no recovery. That's not a bug, that's how Bitcoin works.

## Peer Discovery

Uses IRC bootstrap, same as original Bitcoin. Connects to `irc.libera.chat` and finds other nodes in #bitok.

If IRC is down, you can manually add peers:
```bash
./bitokd -addnode=<ip>
```

## What This Is Not

- Not trying to replace Bitcoin
- Not a fork of BTC (different genesis entirely)
- Not promising you'll get rich
- Not going to add features or "improve" the protocol
- Not going to have a foundation, governance, or roadmap

This is software. It runs. Run it or don't.

## Security

This is 2010 code adapted for 2024. The cryptography is fine (ECDSA, SHA-256 for non-mining hashes). The networking and RPC are... from 2010.

Don't put your life savings in this. Don't run it on a machine you care about without understanding what you're doing. Don't blame me if something goes wrong.

All the security fixes from Satoshi's final release are included:
- Value overflow protection (184B coin bug)
- Blockchain checkpoints
- DoS limits
- IsStandard() filtering

## Documentation

- [MANIFESTO.md](MANIFESTO.md) - why this exists
- [RPC_API.md](RPC_API.md) - JSON-RPC API reference
- [BITOKPOW.md](BITOKPOW.md) - Yespower mining details
- [CHANGELOG.md](CHANGELOG.md) - version history

## License

MIT, same as original Bitcoin. See [license.txt](license.txt).

## Author

Tom Elvis Jedusor

(It's an anagram. Don't worry about it.)

---

> Writing a description for this thing is bloody hard. There's nothing to quite relate it to.

*- Satoshi Nakamoto, January 2009*

Still true.

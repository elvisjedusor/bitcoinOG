# Solo Mining Guide for Bitok

This guide explains how to mine Bitok solo using either the built-in miner or external CPU miners like cpuminer.

---

## Overview

Bitok uses **Yespower 1.0**, a CPU-friendly, memory-hard proof-of-work algorithm. Solo mining means you mine directly to your own wallet without going through a mining pool.

**Key Points:**
- All block rewards (50 BITOK) go directly to you
- You need to find a complete block to get any reward
- Higher hashrate increases your chances of finding blocks
- No pool fees

---

## Method 1: Built-in Miner (Simplest)

The easiest way to solo mine is using Bitok's built-in miner.

### Start Mining

```bash
# Start node and mine with all CPU cores
./bitokd -gen

# Start node and mine with specific number of cores
./bitokd -gen -genproclimit=4

# Run in background
./bitokd -gen -daemon
```

### Control Mining via RPC

```bash
# Enable mining with 4 threads
./bitokd setgenerate true 4

# Disable mining
./bitokd setgenerate false

# Check mining status
./bitokd getmininginfo
```

### Monitor Mining

```bash
# View mining info
./bitokd getmininginfo

# Check network difficulty
./bitokd getdifficulty

# Check your balance
./bitokd getbalance
```

The built-in miner displays hashrate every 30 seconds in the console/log.

---

## Method 2: cpuminer-opt (Recommended for Performance)

[cpuminer-opt](https://github.com/JayDDee/cpuminer-opt) is an optimized CPU miner that supports Yespower and typically achieves higher hashrates than the built-in miner.

### Step 1: Install cpuminer-opt

#### Ubuntu/Debian

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential automake libssl-dev libcurl4-openssl-dev \
    libjansson-dev libgmp-dev zlib1g-dev git

# Clone repository
git clone https://github.com/JayDDee/cpuminer-opt.git
cd cpuminer-opt

# Build with optimizations
./autogen.sh
CFLAGS="-O3 -march=native" ./configure --with-curl
make -j$(nproc)
```

#### macOS

```bash
# Install Homebrew if not installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install automake autoconf openssl curl jansson gmp

# Clone and build
git clone https://github.com/JayDDee/cpuminer-opt.git
cd cpuminer-opt
./autogen.sh
CFLAGS="-O3 -march=native" ./configure --with-curl --with-crypto=/opt/homebrew/opt/openssl
make -j$(sysctl -n hw.ncpu)
```

#### Windows

Download pre-built binaries from:
https://github.com/JayDDee/cpuminer-opt/releases

Or build using MSYS2/MinGW.

### Step 2: Configure Bitok Node

Create or edit `bitok.conf` in your data directory:

| OS | Config Path |
|----|-------------|
| Linux | `~/.bitokd/bitok.conf` |
| macOS | `~/Library/Application Support/Bitok/bitok.conf` |
| Windows | `%APPDATA%\Bitok\bitok.conf` |

```ini
# Enable RPC server
server=1

# RPC credentials (use strong passwords!)
rpcuser=miner
rpcpassword=YourSecurePasswordHere123

# RPC settings
rpcport=8332
rpcallowip=127.0.0.1

# Disable built-in mining (using external miner)
gen=0
```

### Step 3: Start Bitok Node

```bash
# Start daemon (reads config file automatically)
./bitokd -daemon

# Verify node is running
./bitokd getinfo
```

Wait for the node to sync with the network before mining.

### Step 4: Start cpuminer-opt

```bash
./cpuminer \
    -a yespower \
    -o http://127.0.0.1:8332 \
    -u miner \
    -p YourSecurePasswordHere123 \
    --param-n=2048 \
    --param-r=32 \
    --param-key="BitokPoW" \
    -t 4
```

**Parameters Explained:**

| Parameter | Value | Description |
|-----------|-------|-------------|
| `-a` | `yespower` | Algorithm |
| `-o` | `http://127.0.0.1:8332` | Bitok RPC URL |
| `-u` | `miner` | RPC username |
| `-p` | `YourSecurePasswordHere123` | RPC password |
| `--param-n` | `2048` | Yespower N parameter (memory cost) |
| `--param-r` | `32` | Yespower r parameter (block size) |
| `--param-key` | `"BitokPoW"` | Personalization string |
| `-t` | `4` | Number of threads (optional) |


## Performance Tuning

### Thread Count

- **Physical cores only:** Often best for Yespower
- **All cores (with hyperthreading):** May or may not help
- Test both configurations to find optimal

```bash
# Test with physical cores only
./cpuminer ... -t 4

# Test with hyperthreading
./cpuminer ... -t 8
```

### CPU Governor (Linux)

Set CPU to performance mode:

```bash
# Check current governor
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Set to performance
sudo cpupower frequency-set -g performance

# Or for all cores
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

## Monitoring

### Check Mining Status

```bash
# Mining info
./bitokd getmininginfo

# Network hashrate
./bitokd getmininginfo | grep networkhashps

# Difficulty
./bitokd getdifficulty
```

## Troubleshooting

### Connection Refused

```
Error: Connection refused
```

**Solution:** Ensure Bitok node is running with RPC enabled:
```bash
./bitokd -server -daemon
# or with config file containing server=1
```

### Authentication Failed

```
Error: 401 Unauthorized
```

**Solution:** Check RPC username and password match in both config file and miner command.

### Wrong Yespower Parameters

If your miner doesn't support `--param-key`, it won't produce valid hashes. Verify your miner supports Yespower with personalization strings.

**Required parameters:**
- N = 2048
- r = 32
- personalization = "BitokPoW"

### Low Hashrate

1. Check CPU governor is set to performance
2. Verify no thermal throttling
3. Try different thread counts
4. Rebuild cpuminer with `-march=native` for your CPU

### No Blocks Found

Solo mining is probabilistic. With low hashrate relative to network:
- Blocks can take hours or days
- Consider joining a mining pool for more consistent (but smaller) rewards
- Verify you're actually submitting work (check debug.log)

---

## Security Considerations

### RPC Security

- Use strong passwords (32+ characters)
- Never expose RPC to the internet without VPN/firewall
- Use `rpcallowip` to restrict access

### Wallet Backup

Before mining:
```bash
# Backup wallet
cp ~/.bitokd/wallet.dat ~/wallet_backup.dat
```

Mining rewards require 100 confirmations to spend. Don't lose your wallet!

---

## Related Documentation

- [BITOKPOW.md](BITOKPOW.md) - Yespower algorithm details
- [MINING_OPTIMIZATIONS.md](MINING_OPTIMIZATIONS.md) - Built-in miner optimizations
- [RPC_API.md](RPC_API.md) - Complete RPC reference
- [POOL_INTEGRATION.md](POOL_INTEGRATION.md) - For pool operators

---

**Happy Mining!**

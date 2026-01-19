# Mining Pool Integration Guide

This guide provides detailed instructions for mining pool operators to integrate Bitok into their pool infrastructure.

---

## Overview

Bitok uses **Yespower 1.0** proof-of-work, which is CPU-optimized and memory-hard. Pool integration requires:

1. A Bitok node with RPC enabled
2. Pool software that supports Yespower
3. Proper configuration of Yespower parameters

---

## Technical Specifications

### Network Parameters

| Parameter | Value | Source |
|-----------|-------|--------|
| Algorithm | Yespower 1.0 | `yespower_hash.h` |
| Block time | 10 minutes (600 seconds) | `main.cpp:929` |
| Block reward | 50 BITOK (halving every 210,000 blocks) | `main.cpp:919-921` |
| P2P port | 18333 | `net.h:15` |
| RPC port | 8332 (default) | `rpc.cpp:1752` |
| Coinbase maturity | 100 blocks | `main.h:24` |
| Max block size | 1,000,000 bytes | `main.h:19` |
| Network magic | 0xb40bc0de | `net.h:55` |
| Genesis nBits | 0x1effffff | `main.cpp:1733` |
| Difficulty retarget | Every 2016 blocks | `main.cpp:930` |
| Address version | 0 (addresses start with "1") | `base58.h:155` |

### Yespower Parameters

These parameters MUST be used exactly:

| Parameter | Value | Source |
|-----------|-------|--------|
| Version | YESPOWER_1_0 | `yespower_hash.h:30` |
| N | 2048 | `yespower_hash.h:23` |
| r | 32 | `yespower_hash.h:24` |
| pers | "BitokPoW" | `yespower_hash.h:26` |
| perslen | 8 | `yespower_hash.h:27` |

**Memory per hash:** ~128KB per thread (actual memory depends on yespower implementation)

### Block Header Structure

The block header is 80 bytes:

| Field | Size | Description |
|-------|------|-------------|
| nVersion | 4 bytes | Block version (currently 1) |
| hashPrevBlock | 32 bytes | Previous block hash |
| hashMerkleRoot | 32 bytes | Merkle root of transactions |
| nTime | 4 bytes | Block timestamp |
| nBits | 4 bytes | Compact difficulty target |
| nNonce | 4 bytes | Mining nonce |

---

## Node Configuration

### Install and Configure Bitok Node

#### Step 1: Build the Daemon

```bash
# Clone repository
git clone https://github.com/elvisjedusor/bitok.git
cd bitok

# Build daemon
make -f makefile.unix daemon

# Or for optimized mining performance
make -f makefile.unix daemon YESPOWER_ARCH=native
```

#### Step 2: Create Configuration File

Create `bitok.conf` in the data directory:

| OS | Path |
|----|------|
| Linux | `~/.bitokd/bitok.conf` |
| macOS | `~/Library/Application Support/Bitok/bitok.conf` |
| Windows | `%APPDATA%\Bitok\bitok.conf` |

**Pool Node Configuration:**

```ini
# Enable RPC server
server=1

# RPC credentials (use strong passwords!)
rpcuser=poolrpc
rpcpassword=VeryStrongPoolPassword123456789

# RPC settings
rpcport=8332
rpcbind=127.0.0.1

# Allow connections from pool software
rpcallowip=127.0.0.1
rpcallowip=192.168.1.0/24

# Disable built-in mining (pool handles this)
gen=0

# addnode=
# addnode=
```

#### Step 3: Start the Node

```bash
# Start daemon
./bitokd -daemon

# Check status
./bitokd getinfo

# Verify RPC is working
./bitokd getblockcount
```

---

## RPC API for Pools

### Essential RPC Methods

#### getblocktemplate

Returns block template for mining. Implements BIP 22.

```bash
./bitokd getblocktemplate
```

**Response:**
```json
{
  "version": 1,
  "previousblockhash": "000007a5d9c7b6e7b8c9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1",
  "transactions": [
    {
      "data": "0100000001...",
      "hash": "abc123...",
      "fee": 10000
    }
  ],
  "coinbaseaux": {
    "flags": ""
  },
  "coinbasevalue": 5000000000,
  "target": "00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
  "mintime": 1609459200,
  "mutable": ["time", "transactions", "prevblock"],
  "noncerange": "00000000ffffffff",
  "sigoplimit": 20000,
  "sizelimit": 1000000,
  "curtime": 1609459260,
  "bits": "1effffff",
  "height": 12451
}
```

**Key Fields for Pool:**
- `previousblockhash`: Build on this block
- `coinbasevalue`: Maximum reward (in satoshis)
- `target`: Target hash for valid block
- `bits`: Compact target for block header
- `height`: Block height being mined
- `transactions`: Include in block (optional)

#### submitblock

Submit a solved block.

```bash
./bitokd submitblock "0100000000000000...hex_block_data..."
```

**Response:**
- `null` - Block accepted
- `"rejected"` - Block rejected
- Error message - Specific failure reason

#### getmininginfo

Get current mining status.

```bash
./bitokd getmininginfo
```

**Response:**
```json
{
  "blocks": 12450,
  "currentblocksize": 1250,
  "currentblocktx": 3,
  "difficulty": 1.52587891,
  "networkhashps": 125000,
  "pooledtx": 5,
  "chain": "main"
}
```

#### getwork (Legacy)

For older stratum proxies that use getwork.

```bash
./bitokd getwork
```

**Response:**
```json
{
  "data": "0100000000000000...256_hex_chars...",
  "target": "00000000ffff...64_hex_chars...",
  "algorithm": "yespower"
}
```

**Note:** The `data` field is byte-swapped in 4-byte chunks for compatibility.

---

## Pool Software Integration

### Stratum Protocol

Most modern pools use Stratum. Here's how to configure popular pool software:

#### NOMP (Node Open Mining Portal)

Install NOMP:
```bash
git clone https://github.com/zone117x/node-open-mining-portal.git nomp
cd nomp
npm install
```

Create `pool_configs/bitok.json`:

```json
{
    "enabled": true,
    "coin": "bitok.json",
    "address": "YOUR_POOL_BITOK_ADDRESS",
    "rewardRecipients": {
        "YOUR_FEE_ADDRESS": 1.0
    },
    "paymentProcessing": {
        "enabled": true,
        "paymentInterval": 600,
        "minimumPayment": 1,
        "daemon": {
            "host": "127.0.0.1",
            "port": 8332,
            "user": "poolrpc",
            "password": "VeryStrongPoolPassword123456789"
        }
    },
    "ports": {
        "3333": {
            "diff": 0.001,
            "varDiff": {
                "minDiff": 0.0001,
                "maxDiff": 16,
                "targetTime": 15,
                "retargetTime": 90,
                "variancePercent": 30
            }
        }
    },
    "daemons": [
        {
            "host": "127.0.0.1",
            "port": 8332,
            "user": "poolrpc",
            "password": "VeryStrongPoolPassword123456789"
        }
    ],
    "p2p": {
        "enabled": false
    }
}
```

Create `coins/bitok.json`:

```json
{
    "name": "Bitok",
    "symbol": "BITOK",
    "algorithm": "yespower",
    "peerMagic": "b40bc0de",
    "peerMagicNumber": 3020980446,

    "mainnet": {
        "pubKeyHash": 0,
        "wif": 128
    },

    "txMessages": false,

    "yespower": {
        "N": 2048,
        "r": 32,
        "pers": "BitokPoW"
    }
}
```

**Important Notes:**
- `peerMagic`: Network magic bytes from net.h: `{ 0xb4, 0x0b, 0xc0, 0xde }` = "b40bc0de"
- `pubKeyHash`: 0 (from base58.h ADDRESSVERSION) - addresses start with "1"
- `wif`: 128 (0x80) - standard for pubKeyHash=0
- Bitok is based on Bitcoin 0.3.19 which does NOT support:
  - P2SH (BIP 16) - no scriptHash addresses
  - BIP32 HD wallets - no extended keys
- If your pool software requires these fields, use Bitcoin mainnet defaults (scriptHash=5, bip32 public=0x0488B21E) but note they won't be used
- You MUST add Yespower support to NOMP's stratum module (not included by default)

#### MPOS (Mining Portal Open Source)

For MPOS with Stratum mining, configure `stratum/config.php`:

```php
<?php
$config = array(
    'db' => array(
        'host' => 'localhost',
        'user' => 'mpos',
        'pass' => 'password',
        'name' => 'bitok_pool'
    ),
    'wallet' => array(
        'type' => 'http',
        'host' => '127.0.0.1:8332',
        'username' => 'poolrpc',
        'password' => 'VeryStrongPoolPassword123456789'
    ),
    'algorithm' => 'yespower',
    'confirmations' => 100,
    'yespower_n' => 2048,
    'yespower_r' => 32,
    'yespower_pers' => 'BitokPoW'
);
```

**Source references:**
- RPC port 8332: `rpc.cpp:1752`
- Confirmations 100: `main.h:24` (COINBASE_MATURITY)

#### Yiimp

Configure `serverconfig.php`:

```php
define('YAAMP_ALGO', 'yespower');

$bitok_cfg = array(
    'coin' => 'BITOK',
    'algo' => 'yespower',
    'rpchost' => '127.0.0.1',
    'rpcport' => 8332,
    'rpcuser' => 'poolrpc',
    'rpcpassword' => 'VeryStrongPoolPassword123456789',
    'reward' => 50,
    'blocktime' => 600,
    'confirmations' => 100,
    'diff_retarget' => 2016,
    'yespower_n' => 2048,
    'yespower_r' => 32,
    'yespower_pers' => 'BitokPoW'
);
```

**Source references:**
- Block reward 50: `main.cpp:919` (`int64 nSubsidy = 50 * COIN`)
- Block time 600s: `main.cpp:929` (`nTargetSpacing = 10 * 60`)
- Confirmations 100: `main.h:24` (COINBASE_MATURITY)
- Retarget interval 2016: `main.cpp:930` (`nInterval = nTargetTimespan / nTargetSpacing`)

---

## Custom Pool Implementation

### Block Construction

1. **Get block template:**
```python
template = rpc.call('getblocktemplate')
```

2. **Create coinbase transaction:**
```python
def create_coinbase(height, reward, pool_address, extra_nonce):
    tx = Transaction()

    # Coinbase input
    tx.vin.append(TxIn(
        prevout=OutPoint(null_hash, 0xffffffff),
        scriptSig=Script([
            encode_height(height),
            extra_nonce,
            pool_tag
        ])
    ))

    # Output to pool address
    tx.vout.append(TxOut(
        value=reward,
        scriptPubKey=address_to_script(pool_address)
    ))

    return tx
```

3. **Build merkle root:**
```python
def build_merkle_root(coinbase_hash, tx_hashes):
    hashes = [coinbase_hash] + tx_hashes

    while len(hashes) > 1:
        if len(hashes) % 2 == 1:
            hashes.append(hashes[-1])

        next_level = []
        for i in range(0, len(hashes), 2):
            combined = hash256(hashes[i] + hashes[i+1])
            next_level.append(combined)
        hashes = next_level

    return hashes[0]
```

4. **Construct block header:**
```python
def create_block_header(template, merkle_root, nonce, timestamp):
    header = bytearray(80)

    # Version (4 bytes, little-endian)
    header[0:4] = struct.pack('<I', template['version'])

    # Previous block hash (32 bytes)
    header[4:36] = bytes.fromhex(template['previousblockhash'])[::-1]

    # Merkle root (32 bytes)
    header[36:68] = merkle_root[::-1]

    # Timestamp (4 bytes, little-endian)
    header[68:72] = struct.pack('<I', timestamp)

    # Bits (4 bytes, little-endian)
    header[72:76] = bytes.fromhex(template['bits'])[::-1]

    # Nonce (4 bytes, little-endian)
    header[76:80] = struct.pack('<I', nonce)

    return bytes(header)
```

5. **Hash with Yespower:**
```python
import yespower

def hash_block_header(header):
    params = yespower.Params(
        version=yespower.YESPOWER_1_0,
        N=2048,
        r=32,
        pers=b"BitokPoW"
    )
    return yespower.hash(header, params)
```

6. **Submit if valid:**
```python
def submit_block(block_hex):
    result = rpc.call('submitblock', [block_hex])
    return result is None  # None means success
```

### Share Validation

For pool share validation at lower difficulty:

```python
def validate_share(header, share_target):
    hash_result = hash_block_header(header)
    hash_int = int.from_bytes(hash_result, 'little')
    target_int = int.from_bytes(share_target, 'little')
    return hash_int <= target_int
```

### Difficulty Calculation

**IMPORTANT:** Bitok uses a different proof-of-work limit than Bitcoin!

- Bitok: `bnProofOfWorkLimit = ~uint256(0) >> 17` (genesis nBits: 0x1effffff)
- Bitcoin: `bnProofOfWorkLimit = ~uint256(0) >> 32`

```python
def difficulty_to_target(difficulty):
    # Bitok max target: ~uint256(0) >> 17
    # This is 0x00007FFF... (not Bitcoin's 0x00000000FFFF...)
    BITOK_MAX_TARGET = 0x00007FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    target = BITOK_MAX_TARGET // int(difficulty)
    return target

def target_to_difficulty(target):
    # Bitok max target from bnProofOfWorkLimit (~uint256(0) >> 17)
    BITOK_MAX_TARGET = 0x00007FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    return BITOK_MAX_TARGET / target

def bits_to_target(bits):
    # Convert compact "bits" format to target
    # bits = 0x1effffff means exponent=0x1e, mantissa=0x7fffff
    exponent = (bits >> 24) & 0xff
    mantissa = bits & 0x007fffff
    if exponent <= 3:
        target = mantissa >> (8 * (3 - exponent))
    else:
        target = mantissa << (8 * (exponent - 3))
    return target
```

---

## Stratum Protocol Messages

### Mining Subscribe

**Request:**
```json
{
    "id": 1,
    "method": "mining.subscribe",
    "params": ["cpuminer/2.5.1"]
}
```

**Response:**
```json
{
    "id": 1,
    "result": [
        ["mining.notify", "subscription_id"],
        "extranonce1",
        4
    ],
    "error": null
}
```

### Mining Authorize

**Request:**
```json
{
    "id": 2,
    "method": "mining.authorize",
    "params": ["worker_name", "password"]
}
```

### Mining Notify

**Server to client:**
```json
{
    "id": null,
    "method": "mining.notify",
    "params": [
        "job_id",
        "prevhash",
        "coinbase1",
        "coinbase2",
        ["merkle_branch"],
        "version",
        "nbits",
        "ntime",
        true
    ]
}
```

### Mining Submit

**Client to server:**
```json
{
    "id": 3,
    "method": "mining.submit",
    "params": [
        "worker_name",
        "job_id",
        "extranonce2",
        "ntime",
        "nonce"
    ]
}
```

---

## Payment Processing

### PPLNS (Pay Per Last N Shares)

Recommended for Bitok pools due to 10-minute block time.

```python
def calculate_pplns_rewards(block_reward, shares, n_shares):
    # Get last N shares
    recent_shares = shares[-n_shares:]
    total_difficulty = sum(s.difficulty for s in recent_shares)

    rewards = {}
    for share in recent_shares:
        worker = share.worker
        proportion = share.difficulty / total_difficulty
        if worker not in rewards:
            rewards[worker] = 0
        rewards[worker] += block_reward * proportion

    return rewards
```

### Payout Configuration

- **Minimum payout:** 1 BITOK recommended
- **Payout interval:** Every 6 blocks (60 minutes) or on threshold
- **Maturity wait:** 100 confirmations required
- **Transaction fee:** 0.0001 BITOK per payout transaction

---

## Monitoring and Maintenance

### Health Checks

```bash
#!/bin/bash
# pool_health_check.sh

# Check node sync
HEIGHT=$(./bitokd getblockcount)
if [ -z "$HEIGHT" ]; then
    echo "ERROR: Cannot connect to Bitok node"
    exit 1
fi

# Check peer connections
PEERS=$(./bitokd getconnectioncount)
if [ "$PEERS" -lt 3 ]; then
    echo "WARNING: Low peer count: $PEERS"
fi

# Check mempool
MEMPOOL=$(./bitokd getrawmempool | wc -l)
echo "Status: Height=$HEIGHT Peers=$PEERS Mempool=$MEMPOOL"
```

### Log Monitoring

Monitor `debug.log` in data directory for:
- Block submissions
- Rejected blocks
- Network issues
- RPC errors

```bash
tail -f ~/.bitokd/debug.log | grep -E "(block|error|reject)"
```

---

## Security Recommendations

### Network Security

1. **Firewall Rules:**
```bash
# Allow P2P only from trusted peers
iptables -A INPUT -p tcp --dport 18333 -s trusted_ip -j ACCEPT
iptables -A INPUT -p tcp --dport 18333 -j DROP

# Allow RPC only from pool servers
iptables -A INPUT -p tcp --dport 8332 -s pool_server_ip -j ACCEPT
iptables -A INPUT -p tcp --dport 8332 -j DROP
```

2. **RPC Security:**
- Use `rpcallowip` to whitelist pool servers
- Use strong passwords (32+ characters)
- Never expose RPC to public internet

### Redundancy

- Run multiple Bitok nodes for failover
- Use load balancer for RPC requests
- Regular wallet backups
- Monitor for chain splits

---

## Troubleshooting

### Common Issues

**Block Rejected - Bad POW:**
- Verify Yespower parameters (N=2048, r=32, pers="BitokPoW")
- Check block header construction
- Verify hash byte order

**Stale Shares:**
- Reduce `getblocktemplate` polling interval
- Implement long polling
- Check network latency to node

**Payment Failures:**
- Verify wallet has sufficient balance
- Check coinbase maturity (100 blocks)
- Verify address format

**High Orphan Rate:**
- Improve network connectivity
- Add more peers
- Check for chain splits

---

## Testing Checklist

Before going live:

- [ ] Bitok node fully synced
- [ ] RPC authentication working
- [ ] getblocktemplate returns valid data
- [ ] Yespower hashing produces correct results
- [ ] Share validation working at pool difficulty
- [ ] Block submission successful in testnet
- [ ] Payment processing tested
- [ ] Monitoring and alerts configured
- [ ] Backup and recovery tested

---

## Support

For integration assistance:
- GitHub Issues: https://github.com/elvisjedusor/bitok/issues
- Review `rpc.cpp` for RPC implementation details
- Review `main.cpp:3257` for block creation

---

## Related Documentation

- [RPC_API.md](RPC_API.md) - Complete RPC reference
- [RPC_MINING_IMPLEMENTATION.md](RPC_MINING_IMPLEMENTATION.md) - Mining RPC details
- [BITOKPOW.md](BITOKPOW.md) - Yespower algorithm specification
- [SOLO_MINING.md](SOLO_MINING.md) - Solo mining guide

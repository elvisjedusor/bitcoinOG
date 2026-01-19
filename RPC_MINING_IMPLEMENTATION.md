# Bitok RPC Mining API Implementation

## Implemented Components

### Core Infrastructure (util.cpp, init.cpp)
- Configuration file parser (`bitok.conf`)
- `ReadConfigFile()` function (util.cpp:485)
- `ParseParameters()` function (util.cpp:461)
- `GetArg()`, `GetIntArg()`, `GetBoolArg()` helpers (util.cpp:537-560)
- Platform-specific data directory paths (util.cpp:654-724)

### RPC Security (rpc.cpp)
- HTTP Basic Authentication
- Base64 decoder for auth headers
- IP whitelist with CIDR notation support
- Configurable RPC port (`-rpcport`, default 8332)
- Configurable bind address (`-rpcbind`, default 127.0.0.1)
- HTTP 401/403 status codes

### Mining Helper Functions (main.cpp)
- `CreateNewBlock(CKey& key)` - Block template creation (line 3257)
- `GetNetworkHashPS(int lookup)` - Network hashrate estimation (line 3328)

### Mining RPC Methods (rpc.cpp)

| Method | Line | Description |
|--------|------|-------------|
| `getmininginfo` | 452 | Mining statistics and status |
| `getblocktemplate` | 473 | BIP 22 block template |
| `submitblock` | 540 | Submit mined blocks |
| `getwork` | 571 | Legacy mining protocol |

### RPC Dispatch Table (rpc.cpp line 1482-1485)
All methods registered in `pCallTable`

---

## Configuration

### Configuration File

Create `bitok.conf` in your data directory:

```ini
# bitok.conf
server=1
rpcuser=miner
rpcpassword=YourSecurePassword123
rpcport=8332
rpcallowip=127.0.0.1
rpcallowip=192.168.1.0/24
gen=0
```

### Data Directory Locations

| Platform | Path |
|----------|------|
| Linux (daemon) | `~/.bitokd/bitok.conf` |
| macOS | `~/Library/Application Support/Bitok/bitok.conf` |
| Windows | `%APPDATA%\Bitok\bitok.conf` |

---

## RPC Methods Detail

### getmininginfo

Returns comprehensive mining information.

**Response:**
```json
{
  "blocks": 12450,
  "currentblocksize": 0,
  "currentblocktx": 0,
  "difficulty": 1.52587891,
  "networkhashps": 125000,
  "pooledtx": 5,
  "chain": "main",
  "generate": true,
  "genproclimit": 4
}
```

### getblocktemplate

Returns block template for pool mining (BIP 22 compatible).

**Response:**
```json
{
  "version": 1,
  "previousblockhash": "000007a5...",
  "transactions": [],
  "coinbaseaux": {"flags": ""},
  "coinbasevalue": 5000000000,
  "target": "00000000ffff...",
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

### submitblock

Submit a mined block to the network.

**Parameters:**
- `hexdata` (string) - Hex-encoded block data

**Response:**
- `null` on success
- `"rejected"` on failure

### getwork

Legacy getwork protocol for external miners.

**Response (get work):**
```json
{
  "data": "01000000...00000000",
  "target": "00000000ffff...00000000",
  "algorithm": "yespower"
}
```

The `algorithm` field indicates Yespower proof-of-work is required.

**Parameters for Yespower:**
- N (memory cost): 2048
- r (block size): 32
- Personalization: "BitokPoW"

---

## Testing Commands

### Testing RPC Commands

With `bitok.conf` configured (see above), simply run:

```bash
# Start daemon (reads bitok.conf automatically)
./bitokd -daemon

# Test mining info
./bitokd getmininginfo

# Test block template
./bitokd getblocktemplate

# Test getwork
./bitokd getwork
```

### External Miner Testing

```bash
# cpuminer-opt with Yespower
cpuminer -a yespower \
  -o http://127.0.0.1:8332 \
  -u miner -p pass \
  --param-n=2048 --param-r=32 --param-key="BitokPoW"
```

---

## Source Files

| File | Purpose |
|------|---------|
| `util.h` | GetArg helpers, ReadConfigFile declaration |
| `util.cpp` | Config parser, parameter handling |
| `init.cpp` | Config loading on startup |
| `main.h` | CreateNewBlock, GetNetworkHashPS declarations |
| `main.cpp` | CreateNewBlock, GetNetworkHashPS implementations |
| `rpc.cpp` | Mining RPC methods, pCallTable entries |

---

## Backward Compatibility

- No consensus changes
- No protocol changes
- No fork required
- Old nodes work with new nodes
- Same block format (80 bytes)
- Same Yespower PoW parameters

---

## Related Documentation

- [RPC_API.md](RPC_API.md) - Complete RPC API reference
- [SOLO_MINING.md](SOLO_MINING.md) - Solo mining guide with cpuminer
- [POOL_INTEGRATION.md](POOL_INTEGRATION.md) - Mining pool integration guide
- [BITOKPOW.md](BITOKPOW.md) - Yespower proof-of-work details

---

**Implementation Date**: 2026-01-19
**Status**: Ready for use

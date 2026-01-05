# BitcoinOG Implementation Plan

## Project Overview
Deploy a live, functional blockchain network based on Satoshi Nakamoto's v0.3.0-era codebase, creating an independent sovereign network with modified parameters for sustainability while preserving the original code style and architecture.

---

## Phase 1: Ubuntu 24.04 Daemon (Primary Target)

### 1.1 Network Identity (Hard Fork)
**Files to modify:** `net.h`

| Parameter | Original Bitcoin | BitcoinOG |
|-----------|-----------------|-----------|
| `pchMessageStart` | `0xf9, 0xbe, 0xb4, 0xd9` | New unique 4-byte identifier (e.g., `0xb4, 0x0g, 0x20, 0x24`) |
| `DEFAULT_PORT` (P2P) | `8333` (0x8d20) | New port (e.g., `18333` or `8334`) |
| RPC Port | `8332` | New port (e.g., `18332` or `8335`) |

**Purpose:** Absolute network isolation from Bitcoin mainnet.

---

### 1.2 Genesis Block Creation
**Files to modify:** `main.cpp`, `main.h`

**Current Genesis Block (Bitcoin):**
- Hash: `0x000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f`
- Merkle Root: `0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b`
- Timestamp: `1231006505` (Jan 3, 2009)
- nBits: `0x1d00ffff`
- nNonce: `2083236893`
- Coinbase message: "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks"

**New Genesis Block Requirements:**
1. Keep same coinbase message structure
2. New timestamp (same as original OR current launch date)
3. Lower initial difficulty (nBits): `0x1f00ffff` or equivalent for 1 VPS core mining
4. Mine new nonce to satisfy difficulty
5. Calculate and hardcode new genesis hash
6. Calculate and hardcode new merkle root
7. Initial coinbase value: 5 COIN (not 50)

**Code locations (main.cpp:1563-1590):**
```cpp
const uint256 hashGenesisBlock("0x..."); // Line 24 - UPDATE
// Genesis block creation at line 1563-1598
```

---

### 1.3 Difficulty Rules Modification
**Files to modify:** `main.cpp` (function `GetNextWorkRequired` at line 805)

| Parameter | Original Bitcoin | BitcoinOG |
|-----------|-----------------|-----------|
| `nTargetTimespan` | `14 * 24 * 60 * 60` (2 weeks) | `10 * 60` (10 minutes total) |
| `nTargetSpacing` | `10 * 60` (10 minutes) | `60` (1 minute) |
| `nInterval` | `2016 blocks` | `10 blocks` |

**Initial Difficulty:**
- Set `bnProofOfWorkLimit` in `main.h:22` to allow mining 1 block per 30-60 seconds on a single VPS core
- Current: `~uint256(0) >> 32`
- New: Much lower (e.g., `~uint256(0) >> 20` or adjust nBits to `0x1f00ffff`)

---

### 1.4 Block Reward Modification
**Files to modify:** `main.cpp` (function `GetBlockValue` at line 795)

| Parameter | Original Bitcoin | BitcoinOG |
|-----------|-----------------|-----------|
| Initial Reward | `50 * COIN` | `5 * COIN` |
| Halving Interval | `210,000 blocks` | `2,100,000 blocks` (proportionally scaled) |

**Current code:**
```cpp
int64 CBlock::GetBlockValue(int64 nFees) const
{
    int64 nSubsidy = 50 * COIN;
    nSubsidy >>= (nBestHeight / 210000); // Halving
    return nSubsidy + nFees;
}
```

**New code:**
```cpp
int64 CBlock::GetBlockValue(int64 nFees) const
{
    int64 nSubsidy = 5 * COIN;
    nSubsidy >>= (nBestHeight / 2100000); // Halving at 2.1M blocks
    return nSubsidy + nFees;
}
```

---

### 1.5 Seed Node Configuration
**Files to modify:** `net.cpp`

**Current:** `pnSeed[]` array at line 803 contains hardcoded Bitcoin seed node IPs

**Changes needed:**
1. Replace `pnSeed[]` with BitcoinOG VPS node IPs (2-3 nodes)
2. No DNS seeds (remove any DNS-based discovery)
3. Nodes must run 24/7 as permanent bootstrap anchors

**Format:** IP addresses as 32-bit integers in network byte order

---

### 1.6 Branding/Naming Changes
**Files to modify:** Multiple

| Location | Change |
|----------|--------|
| Binary name | `bitcoin` -> `bitcoinogd` |
| Config file | `bitcoin.conf` -> `bitcoinog.conf` |
| Data directory | `.bitcoin` -> `.bitcoinog` |
| RPC messages | Update "bitcoin" references |

**Files affected:**
- `init.cpp` - config file paths
- `util.cpp` - data directory paths
- `rpc.cpp` - RPC messages
- `ui.cpp` - UI strings

---

### 1.7 Build System Modernization
**Files to modify:** `makefile.unix`, potentially new `CMakeLists.txt`

**Target toolchain:**
- Compiler: GCC >= 11
- Standard: C++11 minimum
- OS: Ubuntu 24.04 LTS

**Dependencies:**
- Berkeley DB 4.8 (MANDATORY for wallet compatibility)
- OpenSSL (with compatibility wrappers for deprecated APIs)
- Boost (minimal version for clean compilation)

**Output:** `bitcoinogd` (daemon, static linking preferred)

---

### 1.8 Required RPC Commands (Existing)
**File:** `rpc.cpp`

Verify these work correctly:
- `getinfo` - Line 182
- `getblockcount` - Line 78
- `sendtoaddress` - Exists in mapCallTable
- `getbalance` - Line 134
- `setgenerate` - Line 156 (for mining control)

---

## Phase 2: Ubuntu GUI (wxWidgets)

### 2.1 GUI Compilation
**Files:** `ui.cpp`, `uibase.cpp`, `uibase.h`

- Compile original wxWidgets UI
- Fix only what's required for compilation
- No UI redesign

### 2.2 Required GUI Features
- Create wallet
- Show balance
- Send/receive transactions
- Show transaction history
- Start/stop mining toggle

---

## Phase 3: macOS Port

### 3.1 Targets
- macOS 12+
- Intel + Apple Silicon (Universal Binary)

### 3.2 Changes Required
- wxWidgets compatibility fixes
- Replace deprecated macOS APIs (as needed)
- Codesigning (optional initially)

**Files affected:**
- `makefile.osx`
- Platform-specific code in `net.cpp`, `util.cpp`

---

## Phase 4: Windows 10 Port

### 4.1 Targets
- Windows 10 x64
- MinGW or MSVC build

### 4.2 Deliverables
- GUI wallet
- ZIP distribution (installer optional)

**Files affected:**
- `makefile.mingw`, `makefile.vc`
- `setup.nsi`
- Windows-specific code blocks (`#ifdef __WXMSW__`)

---

## Research Areas (Section 9)

### 9.1 Time Handling
**File:** `util.cpp`

- `GetAdjustedTime()` - potential overflow/drift issues
- Network time synchronization logic
- 32-bit timestamp limitations (Year 2038 problem)

### 9.2 Mining Thread Stability
**File:** `main.cpp` (BitcoinMiner function)

- CPU mining assumptions from 2010
- Thread locking issues on modern multi-core CPUs
- `vnThreadsRunning` array management

### 9.3 Networking
**File:** `net.cpp`

- IPv6 handling (limited in original code)
- Socket initialization modernization
- Non-blocking IO assumptions
- Modern select/poll/epoll considerations

---

## File Modification Summary

### Critical Files (Must Modify)

| File | Changes |
|------|---------|
| `main.h` | `hashGenesisBlock`, `bnProofOfWorkLimit` |
| `main.cpp` | Genesis block, `GetBlockValue`, `GetNextWorkRequired` |
| `net.h` | `pchMessageStart`, `DEFAULT_PORT` |
| `net.cpp` | `pnSeed[]` array |
| `rpc.cpp` | RPC port, branding |
| `init.cpp` | Config paths, branding |
| `util.cpp` | Data directory paths |

### Build Files (Must Modify)

| File | Purpose |
|------|---------|
| `makefile.unix` | Ubuntu 24.04 build |
| `makefile.osx` | macOS build |
| `makefile.mingw` | Windows MinGW build |
| `makefile.vc` | Windows MSVC build |

---

## Success Criteria

1. Fresh node can download client
2. Fresh node can sync from genesis
3. Fresh node can mine blocks
4. Fresh node can send/receive coins
5. Network remains live without manual intervention
6. Multiple independent nodes stay in consensus

---

## Documentation Deliverables

1. `README.md` - Philosophy, build instructions
2. `PROTOCOL_CHANGES.md` - All protocol modifications
3. `NETWORK_PARAMS.md` - Network parameters
4. `DISCLAIMER.md` - Legal/philosophical positioning

---

## Implementation Order

1. **Network Identity** - `pchMessageStart`, ports (isolates from Bitcoin)
2. **Genesis Block** - New genesis with low difficulty
3. **Difficulty Rules** - 1-minute blocks, 10-block retarget
4. **Block Reward** - 5 coins, halving at 2.1M blocks
5. **Seed Nodes** - Hardcode VPS IPs
6. **Build System** - Modern Ubuntu compilation
7. **Testing** - Multi-node consensus verification
8. **GUI** - wxWidgets compilation
9. **Cross-platform** - macOS, Windows ports

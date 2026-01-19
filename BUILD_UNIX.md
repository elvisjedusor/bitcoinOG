# UNIX Build Instructions

Copyright (c) 2009-2010 Satoshi Nakamoto

Copyright (c) 2016 Tom Elvis Jedusor

Distributed under the MIT/X11 software license, see the accompanying file license.txt or http://www.opensource.org/licenses/mit-license.php.

This product includes software developed by the OpenSSL Project for use in the OpenSSL Toolkit (http://www.openssl.org/). This product includes cryptographic software written by Eric Young (eay@cryptsoft.com).

---

## What Is Bitok?

Bitok is Bitcoin v0.3.19 (Satoshi's final release, December 2010) adapted to run on modern systems with **GPU-resistant proof-of-work**.

Bitcoin v0.3.19 includes all critical security fixes from the Satoshi era (value overflow protection, checkpoints, DoS limits, IsStandard filter).

**Key changes from original Bitcoin:**
- ✅ Modern system compatibility (Ubuntu 24.04, OpenSSL 3.x, etc.)
- ✅ Yespower mining algorithm (CPU-friendly, GPU/ASIC-resistant)
- ✅ New genesis block (separate network)
- ✅ All Satoshi-era security fixes included from launch

For mining details, see [BITOKPOW.md](BITOKPOW.md).
For philosophy, see [MANIFESTO.md](MANIFESTO.md).

---

## Overview

This codebase has been modernized to compile on **Ubuntu 24.04 LTS** with modern toolchain:

- **GCC 11+** with C++11 standard
- **OpenSSL 3.x** (with compatibility wrappers for deprecated APIs)
- **Berkeley DB 5.3** (using C API compatibility layer)
- **Boost 1.74+**
- **wxWidgets 3.2** (optional, for GUI)
- **Yespower** (CPU-optimized proof-of-work with automatic SIMD detection)

---

## Dependencies

### Core Dependencies

Install the required packages for building the bitokd daemon:

```bash
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install libssl-dev
sudo apt-get install libdb-dev libdb5.3-dev
sudo apt-get install libboost-all-dev
```

### GUI Dependencies (Optional)

For building the graphical wallet interface:

```bash
sudo apt-get install libwxgtk3.2-dev
sudo apt-get install libgtk-3-dev
```

---

## Building

### Build the Daemon (bitokd)

The daemon is a command-line node that can mine, relay transactions, and provide JSON-RPC interface:

```bash
make -f makefile.unix clean
make -f makefile.unix
```

The daemon binary will be created as `bitokd`.

### Build the GUI Wallet (bitok)

The GUI wallet requires wxWidgets. After installing the GUI dependencies:

```bash
make -f makefile.unix gui
```

The GUI binary will be created as `bitok`.

### Build Both

To build both daemon and GUI in one command:

```bash
make -f makefile.unix all
```

---

## CPU Optimization

The makefile automatically uses `-march=native` to compile with all CPU features available on your system (AVX2, SSE4.1, etc.). This provides maximum performance.

**For distribution binaries:**
```bash
make -f makefile.unix all YESPOWER_ARCH=x86-64-v3  # For modern CPUs (2015+)
make -f makefile.unix all YESPOWER_ARCH=x86-64     # Maximum compatibility
```

---

## Installation

To install the binaries to `/usr/local/bin/`:

```bash
sudo make -f makefile.unix install
```

---

### Run the Daemon

Basic daemon startup:

```bash
./bitokd
```

### Daemon Options

```bash
./bitokd -daemon                           # Run in background
./bitokd --help                            # Show all options
```

Settings like RPC credentials and mining are configured in `bitok.conf` (see Configuration section below).

### RPC Commands

Once the daemon is running, you can use these RPC commands:

```bash
# Information
./bitokd help                              # List all commands
./bitokd getinfo                           # Get general information
./bitokd getblockcount                     # Get number of blocks
./bitokd getblocknumber                    # Get latest block number
./bitokd getconnectioncount                # Get peer connections
./bitokd getdifficulty                     # Get mining difficulty

# Wallet
./bitokd getbalance                        # Get wallet balance
./bitokd getnewaddress [label]             # Generate new address
./bitokd sendtoaddress <address> <amount>  # Send coins

# Address & Label Management
./bitokd setlabel <address> <label>        # Set address label
./bitokd getlabel <address>                # Get address label
./bitokd getaddressesbylabel <label>       # List addresses by label

# Receiving
./bitokd getreceivedbyaddress <address>    # Amount received by address
./bitokd getreceivedbylabel <label>        # Amount received by label
./bitokd listreceivedbyaddress             # List received by address
./bitokd listreceivedbylabel               # List received by label

# Mining
./bitokd getgenerate                       # Check if mining is enabled
./bitokd setgenerate <true/false>          # Enable/disable mining

# System
./bitokd stop                              # Stop daemon
```

### Run the GUI Wallet

```bash
./bitok
```

The GUI provides a user-friendly interface for sending/receiving coins, viewing transactions, and managing your wallet.

---

## Configuration

Create `bitok.conf` in your data directory:

| OS | Config File Path |
|----|------------------|
| Linux | `~/.bitokd/bitok.conf` |
| macOS | `~/Library/Application Support/Bitok/bitok.conf` |

**Example `bitok.conf`:**
```ini
server=1
rpcuser=yourusername
rpcpassword=yourpassword
rpcport=8332
rpcallowip=127.0.0.1
gen=1
```

**Secure the config file:**
```bash
chmod 600 ~/.bitokd/bitok.conf
```

**Usage:**
```bash
./bitokd -daemon                           # Reads bitok.conf automatically
./bitokd getinfo                           # Uses credentials from config
./bitokd --help                            # Show all options
```

---

## Troubleshooting

### OpenSSL Deprecation Warnings

If you encounter OpenSSL errors about deprecated functions, ensure `OPENSSL_SUPPRESS_DEPRECATED` is defined in the makefile (already configured).

### Berkeley DB Errors

The code uses a C API compatibility layer (`db_cxx_compat.h`) instead of the deprecated C++ bindings that are no longer available in modern Ubuntu distributions.

### Missing Dependencies

If you get compilation errors about missing headers:
- Verify all dependencies are installed
- Check that `libdb5.3-dev` is properly installed for Berkeley DB
- Ensure `libwxgtk3.2-dev` is installed if building the GUI

### Runtime Errors

If the daemon fails to start:
- Check that port 8333 is not already in use
- Verify the data directory `~/.bitokd` exists and is writable
- Check logs in `~/.bitokd/debug.log`

---


## Modernization Changes

The following changes were made for Ubuntu 24.04 compatibility:

### 1. OpenSSL 3.x Compatibility
- `BIGNUM` is now opaque in OpenSSL 3.x
- `CBigNum` class refactored to wrap `BIGNUM*` instead of inheriting from `BIGNUM` struct
- Added proper memory management with `BN_new()` and `BN_free()`

### 2. Berkeley DB C API
- C++ bindings (`db_cxx.h`) removed from modern Ubuntu
- Created `db_cxx_compat.h` wrapper using C API
- Maintains compatibility with original code structure

### 3. Boost Updates
- Added `BOOST_BIND_GLOBAL_PLACEHOLDERS` to suppress deprecation warnings
- Updated bind placeholder usage for modern Boost versions

### 4. GCC 11+ Compatibility
- Fixed narrowing conversions
- Added C++11 standard flag
- Resolved deprecated conversion warnings

### 5. wxWidgets Separation
- Daemon build no longer requires wxWidgets
- GUI-specific code conditionally compiled with `#if wxUSE_GUI`
- Clean separation between CLI and GUI builds

### 6. Yespower Proof-of-Work
- Replaced SHA-256 with Yespower 1.0 (N=2048, r=32, personalization="BitokPoW")
- Implements Satoshi's vision of CPU-only mining
- Automatic CPU feature detection (SSE2/AVX/AVX2/AVX512)
- Memory-hard algorithm (~128KB per hash)
- GPU/ASIC-resistant by design

See [BITOKPOW.md](BITOKPOW.md) for complete technical details.

---

## Original Build Environment (Historical)

The original version was built with:

| Component    | Version      |
|--------------|--------------|
| GCC          | 4.4.3        |
| OpenSSL      | 0.9.8k       |
| wxWidgets    | 2.9.0        |
| Berkeley DB  | 4.7.25.NC    |
| Boost        | 1.40.0       |

---

## UI Development

For GUI development, the UI layout is designed with **wxFormBuilder**.

The project file is `uiproject.fbp`.

To modify the GUI:
1. Open `uiproject.fbp` in wxFormBuilder
2. Make your changes
3. Generate code (outputs to `uibase.cpp` and `uibase.h`)
4. Implement custom logic in `ui.cpp` and `ui.h`

---

## JSON-RPC Interface

### Protocol Overview

The daemon provides a **JSON-RPC 1.0** interface for programmatic access. This allows applications to interact with the Bitcoin node over HTTP.

**Specifications:**
- Protocol: JSON-RPC 1.0
- Transport: HTTP/1.1 POST requests
- Default Port: `8332`
- Binding: `127.0.0.1` (localhost only, for security)
- Content-Type: `application/json`

### Authentication

RPC requests require HTTP Basic Authentication. Configure credentials in `bitok.conf`:

```ini
server=1
rpcuser=myuser
rpcpassword=mypassword
```

See the [Configuration](#configuration) section for config file locations.

### Request Format

JSON-RPC requests must be sent as HTTP POST to `http://127.0.0.1:8332/` with this structure:

```json
{
  "method": "command_name",
  "params": [param1, param2, ...],
  "id": 1
}
```

**Fields:**
- `method` (string): The RPC command name
- `params` (array): Array of parameters (can be empty)
- `id` (number/string): Request identifier returned in response

### Response Format

**Success Response:**
```json
{
  "result": <command result>,
  "error": null,
  "id": 1
}
```

**Error Response:**
```json
{
  "result": null,
  "error": "error message",
  "id": 1
}
```

### Complete RPC Command Reference

#### Information Commands

**`help`**
```json
{"method": "help", "params": [], "id": 1}
```
Returns list of all available commands.

**`getinfo`**
```json
{"method": "getinfo", "params": [], "id": 1}
```
Returns object with:
- `balance`: Current wallet balance
- `blocks`: Number of blocks in chain
- `connections`: Number of peer connections
- `proxy`: Proxy settings (if using SOCKS)
- `generate`: Whether mining is enabled
- `genproclimit`: Processor limit for mining (-1 = unlimited)
- `difficulty`: Current mining difficulty

**`getblockcount`**
```json
{"method": "getblockcount", "params": [], "id": 1}
```
Returns total number of blocks in the longest chain.

**`getblocknumber`**
```json
{"method": "getblocknumber", "params": [], "id": 1}
```
Returns block number of the latest block.

**`getconnectioncount`**
```json
{"method": "getconnectioncount", "params": [], "id": 1}
```
Returns number of connections to other nodes.

**`getdifficulty`**
```json
{"method": "getdifficulty", "params": [], "id": 1}
```
Returns proof-of-work difficulty as a multiple of minimum difficulty.

#### Wallet Commands

**`getbalance`**
```json
{"method": "getbalance", "params": [], "id": 1}
```
Returns the wallet's available balance in BTC.

**`getnewaddress [label]`**
```json
{"method": "getnewaddress", "params": ["my_label"], "id": 1}
```
Generates and returns a new Bitcoin address. Optional label can be provided.

**`sendtoaddress <address> <amount> [comment] [comment-to]`**
```json
{
  "method": "sendtoaddress",
  "params": ["1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa", 10.5, "payment", "recipient"],
  "id": 1
}
```
Sends amount (in BTC) to the specified address. Returns transaction status.

#### Address & Label Management

**`setlabel <address> <label>`**
```json
{
  "method": "setlabel",
  "params": ["1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa", "Satoshi"],
  "id": 1
}
```
Associates a label with an address.

**`getlabel <address>`**
```json
{
  "method": "getlabel",
  "params": ["1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"],
  "id": 1
}
```
Returns the label associated with an address.

**`getaddressesbylabel <label>`**
```json
{
  "method": "getaddressesbylabel",
  "params": ["Satoshi"],
  "id": 1
}
```
Returns array of all addresses with the specified label.

#### Receiving Commands

**`getreceivedbyaddress <address> [minconf]`**
```json
{
  "method": "getreceivedbyaddress",
  "params": ["1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa", 6],
  "id": 1
}
```
Returns total amount received by address with at least `minconf` confirmations (default: 1).

**`getreceivedbylabel <label> [minconf]`**
```json
{
  "method": "getreceivedbylabel",
  "params": ["Satoshi", 6],
  "id": 1
}
```
Returns total amount received by all addresses with label.

**`listreceivedbyaddress [minconf] [includeempty]`**
```json
{
  "method": "listreceivedbyaddress",
  "params": [1, false],
  "id": 1
}
```
Returns array of objects with address, label, amount, and confirmations.

**`listreceivedbylabel [minconf] [includeempty]`**
```json
{
  "method": "listreceivedbylabel",
  "params": [1, false],
  "id": 1
}
```
Returns array of objects grouped by label.

**`listtransactions [count] [includegenerated]`**
```json
{
  "method": "listtransactions",
  "params": [10, false],
  "id": 1
}
```
Returns up to `count` most recent transactions. (Note: Not fully implemented in this version)

#### Mining Commands

**`getgenerate`**
```json
{"method": "getgenerate", "params": [], "id": 1}
```
Returns true/false indicating if mining is enabled.

**`setgenerate <generate> [genproclimit]`**
```json
{
  "method": "setgenerate",
  "params": [true, 4],
  "id": 1
}
```
Enable/disable mining. Optionally set processor limit (-1 for unlimited).

#### System Commands

**`stop`**
```json
{"method": "stop", "params": [], "id": 1}
```
Stops the Bitcoin server.

### Example: Making RPC Calls with cURL

**Get Info:**
```bash
curl --user myuser:mypassword --data-binary '{"method":"getinfo","params":[],"id":1}' \
  -H 'content-type: application/json' http://127.0.0.1:8332/
```

**Get Balance:**
```bash
curl --user myuser:mypassword --data-binary '{"method":"getbalance","params":[],"id":1}' \
  -H 'content-type: application/json' http://127.0.0.1:8332/
```

**Generate New Address:**
```bash
curl --user myuser:mypassword --data-binary '{"method":"getnewaddress","params":["savings"],"id":1}' \
  -H 'content-type: application/json' http://127.0.0.1:8332/
```

**Send Transaction:**
```bash
curl --user myuser:mypassword --data-binary \
  '{"method":"sendtoaddress","params":["1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa",10.5],"id":1}' \
  -H 'content-type: application/json' http://127.0.0.1:8332/
```

### Example: Python Client

```python
import requests
import json

class BitcoinRPC:
    def __init__(self, user, password, host='127.0.0.1', port=8332):
        self.url = f'http://{host}:{port}/'
        self.auth = (user, password)
        self.headers = {'content-type': 'application/json'}
        self.id = 0

    def call(self, method, params=[]):
        self.id += 1
        payload = {
            'method': method,
            'params': params,
            'id': self.id
        }
        response = requests.post(
            self.url,
            data=json.dumps(payload),
            headers=self.headers,
            auth=self.auth
        )
        result = response.json()

        if result['error'] is not None:
            raise Exception(result['error'])

        return result['result']

# Usage
rpc = BitcoinRPC('myuser', 'mypassword')

# Get info
info = rpc.call('getinfo')
print(f"Balance: {info['balance']} BTC")
print(f"Blocks: {info['blocks']}")
print(f"Connections: {info['connections']}")

# Get new address
address = rpc.call('getnewaddress', ['savings'])
print(f"New address: {address}")

# Send coins
result = rpc.call('sendtoaddress', [address, 1.5])
print(f"Transaction: {result}")
```

### Error Handling

Common error scenarios:

- **Connection refused:** Daemon not running or RPC server not enabled
- **401 Unauthorized:** Invalid username/password
- **Method not found:** Typo in method name or command not available
- **Invalid params:** Wrong number or type of parameters
- **Parse error:** Malformed JSON request

### Batch Requests

Multiple commands can be sent in a single HTTP request by sending multiple JSON objects separated by newlines:

```bash
echo -e '{"method":"getinfo","params":[],"id":1}\n{"method":"getbalance","params":[],"id":2}' | \
curl --user myuser:mypassword --data-binary @- \
  -H 'content-type: application/json' http://127.0.0.1:8332/
```

---

## Additional Resources

- Original Bitcoin whitepaper: https://bitcoin.org/bitcoin.pdf
- JSON-RPC 1.0 Specification: https://www.jsonrpc.org/specification_v1

---

## License

This software is distributed under the MIT/X11 License. See `license.txt` for details.

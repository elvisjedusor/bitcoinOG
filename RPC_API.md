# Bitok RPC API Documentation

JSON-RPC API for exchanges, mining pools, block explorers, and whatever else you need it for.

Version: Bitok 0.3.19 Mainnet

---

## Table of Contents

1. [Introduction](#introduction)
2. [Connection & Authentication](#connection--authentication)
3. [General Information](#general-information)
4. [Wallet Operations](#wallet-operations)
5. [Transaction Operations](#transaction-operations)
6. [Mining Operations](#mining-operations)
7. [Network Operations](#network-operations)
8. [Block Chain Operations](#block-chain-operations)
9. [Integration Examples](#integration-examples)
10. [Error Handling](#error-handling)
11. [Security Best Practices](#security-best-practices)

---

## Introduction

The Bitok daemon exposes a JSON-RPC 1.0 interface. You can query blockchain data, manage wallets, send transactions, and control mining.

### Protocol Details

| Property | Value |
|----------|-------|
| Protocol | JSON-RPC 1.0 |
| Transport | HTTP POST |
| Default Port | 8332 |
| Authentication | HTTP Basic Auth |
| Content-Type | application/json |

---

## Connection & Authentication

### Everything is command-line flags.

```bash
# Start daemon with RPC enabled
./bitokd -rpcuser=yourusername -rpcpassword=yourpassword -server

# Run in background
./bitokd -rpcuser=yourusername -rpcpassword=yourpassword -server -daemon

# Execute RPC command (uses same credentials)
./bitokd -rpcuser=yourusername -rpcpassword=yourpassword getinfo
./bitokd -rpcuser=yourusername -rpcpassword=yourpassword getbalance
./bitokd -rpcuser=yourusername -rpcpassword=yourpassword getnewaddress "customer123"
```

Yes, you have to type the credentials every time. That's how it worked in 2010.

Alternatively, use environment variables in a wrapper script:

```bash
#!/bin/bash
# bitok-cli.sh
BITOK_USER="yourusername"
BITOK_PASS="yourpassword"
./bitokd -rpcuser=$BITOK_USER -rpcpassword=$BITOK_PASS "$@"
```

### Security Recommendations

1. **Strong Credentials:** Use long, random passwords. 32+ characters if you're paranoid.
2. **Firewall Rules:** Restrict RPC port (8332) to trusted IPs only.
3. **Local Only:** By default, RPC binds to 127.0.0.1. Don't change that unless you know what you're doing.
4. **SSH Tunnel:** For remote access, tunnel through SSH instead of exposing the port.

### Programmatic Access

#### cURL Example

```bash
curl --user yourusername:yourpassword \
     --data-binary '{"jsonrpc":"1.0","id":"1","method":"getinfo","params":[]}' \
     -H 'content-type: application/json;' \
     http://127.0.0.1:8332/
```

#### Python Example

```python
import requests
import json

class BitokRPC:
    def __init__(self, user, password, host='127.0.0.1', port=8332):
        self.url = f'http://{host}:{port}/'
        self.auth = (user, password)
        self.headers = {'content-type': 'application/json'}
        self.id = 0

    def call(self, method, params=[]):
        self.id += 1
        payload = {
            'jsonrpc': '1.0',
            'id': self.id,
            'method': method,
            'params': params
        }
        response = requests.post(
            self.url,
            data=json.dumps(payload),
            headers=self.headers,
            auth=self.auth
        )
        return response.json()['result']

# Usage
rpc = BitokRPC('yourusername', 'yourpassword')
info = rpc.call('getinfo')
print(f"Balance: {info['balance']} BITOK")
print(f"Blocks: {info['blocks']}")
```

#### PHP Example

```php
<?php
class BitokRPC {
    private $url;
    private $user;
    private $pass;
    private $id = 0;

    public function __construct($user, $pass, $host = '127.0.0.1', $port = 8332) {
        $this->url = "http://{$host}:{$port}/";
        $this->user = $user;
        $this->pass = $pass;
    }

    public function call($method, $params = []) {
        $this->id++;
        $request = json_encode([
            'jsonrpc' => '1.0',
            'id' => $this->id,
            'method' => $method,
            'params' => $params
        ]);

        $ch = curl_init($this->url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $request);
        curl_setopt($ch, CURLOPT_USERPWD, "{$this->user}:{$this->pass}");

        $response = curl_exec($ch);
        curl_close($ch);

        return json_decode($response, true)['result'];
    }
}

// Usage
$rpc = new BitokRPC('yourusername', 'yourpassword');
$info = $rpc->call('getinfo');
echo "Balance: {$info['balance']} BITOK\n";
?>
```

#### Node.js Example

```javascript
const axios = require('axios');

class BitokRPC {
    constructor(user, pass, host = '127.0.0.1', port = 8332) {
        this.url = `http://${host}:${port}/`;
        this.auth = {
            username: user,
            password: pass
        };
        this.id = 0;
    }

    async call(method, params = []) {
        this.id++;
        const response = await axios.post(this.url, {
            jsonrpc: '1.0',
            id: this.id,
            method: method,
            params: params
        }, {
            auth: this.auth,
            headers: {'Content-Type': 'application/json'}
        });
        return response.data.result;
    }
}

// Usage
(async () => {
    const rpc = new BitokRPC('yourusername', 'yourpassword');
    const info = await rpc.call('getinfo');
    console.log(`Balance: ${info.balance} BITOK`);
    console.log(`Blocks: ${info.blocks}`);
})();
```

---

## General Information

### help

Lists all available RPC commands.

**Parameters:** None

**Returns:** String containing list of commands

**Example:**
```bash
./bitokd help
```

**Response:**
```
getblockcount
getblocknumber
getblockhash <index>
getblock <hash>
gettransaction <txid>
validateaddress <address>
getconnectioncount
getdifficulty
getbalance
getgenerate
setgenerate <generate> [genproclimit]
getinfo
getnewaddress [label]
...
```

---

### getinfo

Returns general information about the node and wallet.

**Parameters:** None

**Returns:** Object containing:
- `balance` (number) - Total balance in BITOK
- `blocks` (number) - Current block count
- `connections` (number) - Number of peer connections
- `proxy` (string) - Proxy address if configured
- `generate` (boolean) - Mining status
- `genproclimit` (number) - Number of mining threads (-1 = all cores)
- `difficulty` (number) - Current network difficulty

**Example:**
```bash
./bitokd getinfo
```

**Response:**
```json
{
  "balance": 150.00000000,
  "blocks": 12450,
  "connections": 8,
  "proxy": "",
  "generate": true,
  "genproclimit": 4,
  "difficulty": 1.00000000
}
```

**Use Case - Exchange Health Check:**
```python
def check_node_health(rpc):
    info = rpc.call('getinfo')

    # Check if node is synchronized
    if info['blocks'] < expected_height:
        alert("Node behind blockchain")

    # Check peer connectivity
    if info['connections'] < 3:
        alert("Low peer count")

    # Check wallet balance
    if info['balance'] < hot_wallet_minimum:
        alert("Hot wallet needs refill")

    return info
```

---

### stop

Gracefully shuts down the Bitok server.

**Parameters:** None

**Returns:** String confirmation message

**Example:**
```bash
./bitokd stop
```

**Response:**
```
"Bitok server stopping"
```

**Warning:** This will disconnect all RPC clients and stop the daemon. Use with caution in production environments.

---

## Wallet Operations

### getbalance

Returns the total available balance in the wallet.

**Parameters:** None

**Returns:** Number (balance in BITOK)

**Example:**
```bash
./bitokd getbalance
```

**Response:**
```
150.50000000
```

**Use Case - Exchange Hot Wallet Monitoring:**
```python
def monitor_hot_wallet(rpc, threshold=100.0):
    """Monitor hot wallet and alert if balance is low"""
    balance = rpc.call('getbalance')

    if balance < threshold:
        # Trigger cold wallet transfer
        print(f"WARNING: Hot wallet balance {balance} below threshold {threshold}")
        return False

    return True
```

**Note:** Balance includes only confirmed transactions (6+ confirmations by default).

---

### getnewaddress

Generates a new Bitok address for receiving payments.

**Parameters:**
- `label` (string, optional) - Label to associate with the address

**Returns:** String (new Bitok address)

**Example:**
```bash
./bitokd getnewaddress
./bitokd getnewaddress "customer123"
./bitokd getnewaddress "deposit_user_5678"
```

**Response:**
```
"1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"
```

**Use Case - Exchange Deposit Address Generation:**
```python
def generate_deposit_address(rpc, user_id):
    """Generate unique deposit address for user"""
    label = f"deposit_user_{user_id}"
    address = rpc.call('getnewaddress', [label])

    # Store in database
    db.execute(
        "INSERT INTO deposit_addresses (user_id, address, label, created_at) "
        "VALUES (?, ?, ?, ?)",
        (user_id, address, label, datetime.now())
    )

    return address
```

**Best Practice:** Always use descriptive labels for tracking purposes. Labels help identify address purpose during audits.

---

### setlabel

Sets or updates the label associated with a Bitok address.

**Parameters:**
- `address` (string, required) - Bitok address
- `label` (string, required) - Label to assign

**Returns:** null

**Example:**
```bash
./bitokd setlabel "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa" "primary_deposit"
```

**Use Case - Address Labeling System:**
```python
def label_address(rpc, address, purpose, user_id=None):
    """Create structured label for address"""
    if user_id:
        label = f"{purpose}_user_{user_id}"
    else:
        label = purpose

    rpc.call('setlabel', [address, label])
    return label
```

---

### getlabel

Retrieves the label associated with a Bitok address.

**Parameters:**
- `address` (string, required) - Bitok address

**Returns:** String (label or empty string if none)

**Example:**
```bash
./bitokd getlabel "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"
```

**Response:**
```
"customer123"
```

---

### getaddressesbylabel

Returns all addresses associated with a specific label.

**Parameters:**
- `label` (string, required) - Label to search for

**Returns:** Array of addresses

**Example:**
```bash
./bitokd getaddressesbylabel "merchant_deposits"
```

**Response:**
```json
[
  "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa",
  "1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2"
]
```

**Use Case - Batch Address Management:**
```python
def get_all_deposit_addresses(rpc, merchant_id):
    """Get all deposit addresses for a merchant"""
    label = f"merchant_{merchant_id}_deposits"
    addresses = rpc.call('getaddressesbylabel', [label])
    return addresses
```

---

### getreceivedbyaddress

Returns the total amount received by a specific address.

**Parameters:**
- `address` (string, required) - Bitok address
- `minconf` (number, optional, default=1) - Minimum confirmations

**Returns:** Number (amount received in BITOK)

**Example:**
```bash
./bitokd getreceivedbyaddress "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"
./bitokd getreceivedbyaddress "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa" 6
```

**Response:**
```
50.75000000
```

**Use Case - Deposit Detection (Exchange):**
```python
def check_deposits(rpc, address, last_known_amount):
    """Check if new deposits arrived for an address"""
    current_amount = rpc.call('getreceivedbyaddress', [address, 1])

    if current_amount > last_known_amount:
        new_deposit = current_amount - last_known_amount
        print(f"New deposit: {new_deposit} BITOK")

        # Credit user account after confirmations
        if rpc.call('getreceivedbyaddress', [address, 6]) >= current_amount:
            credit_user_account(address, new_deposit)
            return new_deposit

    return 0
```

**Important:** This returns the total amount ever received by the address, not the current balance. Track previous values to detect new deposits.

---

### getreceivedbylabel

Returns the total amount received by all addresses with a specific label.

**Parameters:**
- `label` (string, required) - Label to query
- `minconf` (number, optional, default=1) - Minimum confirmations

**Returns:** Number (total amount received in BITOK)

**Example:**
```bash
./bitokd getreceivedbylabel "merchant_deposits" 6
```

**Response:**
```
250.50000000
```

---

### listreceivedbyaddress

Lists amounts received by each address in the wallet.

**Parameters:**
- `minconf` (number, optional, default=1) - Minimum confirmations
- `includeempty` (boolean, optional, default=false) - Include addresses with zero balance

**Returns:** Array of objects containing:
- `address` (string) - Receiving address
- `label` (string) - Address label
- `amount` (number) - Total amount received
- `confirmations` (number) - Confirmations of most recent transaction

**Example:**
```bash
./bitokd listreceivedbyaddress 6 true
```

**Response:**
```json
[
  {
    "address": "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa",
    "label": "customer123",
    "amount": 50.00000000,
    "confirmations": 145
  },
  {
    "address": "1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2",
    "label": "deposit_user_456",
    "amount": 25.50000000,
    "confirmations": 12
  }
]
```

**Use Case - Exchange Deposit Scanner:**
```python
def scan_all_deposits(rpc, min_confirmations=6):
    """Scan all addresses for confirmed deposits"""
    received = rpc.call('listreceivedbyaddress', [min_confirmations, False])

    deposits = []
    for item in received:
        if item['amount'] > 0:
            deposits.append({
                'address': item['address'],
                'label': item['label'],
                'amount': item['amount'],
                'confirmations': item['confirmations']
            })

    return deposits
```

---

### listreceivedbylabel

Lists amounts received grouped by label.

**Parameters:**
- `minconf` (number, optional, default=1) - Minimum confirmations
- `includeempty` (boolean, optional, default=false) - Include labels with zero balance

**Returns:** Array of objects containing:
- `label` (string) - Label name
- `amount` (number) - Total amount received
- `confirmations` (number) - Confirmations of most recent transaction

**Example:**
```bash
./bitokd listreceivedbylabel 6 false
```

**Response:**
```json
[
  {
    "label": "merchant_deposits",
    "amount": 150.00000000,
    "confirmations": 45
  },
  {
    "label": "customer123",
    "amount": 50.00000000,
    "confirmations": 145
  }
]
```

---

### listunspent

Returns array of unspent transaction outputs (UTXOs) in the wallet.

**Parameters:**
- `minconf` (number, optional, default=1) - Minimum confirmations
- `maxconf` (number, optional, default=9999999) - Maximum confirmations

**Returns:** Array of objects containing:
- `txid` (string) - Transaction ID
- `vout` (number) - Output index
- `address` (string) - Associated address
- `scriptPubKey` (string) - Hex-encoded script
- `amount` (number) - Amount in BITOK
- `confirmations` (number) - Number of confirmations

**Example:**
```bash
./bitokd listunspent
./bitokd listunspent 6
./bitokd listunspent 6 100
```

**Response:**
```json
[
  {
    "txid": "a1b2c3d4e5f6789012345678901234567890123456789012345678901234abcd",
    "vout": 0,
    "address": "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa",
    "scriptPubKey": "76a914...",
    "amount": 50.00000000,
    "confirmations": 145
  },
  {
    "txid": "b2c3d4e5f67890123456789012345678901234567890123456789012345bcde",
    "vout": 1,
    "address": "1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2",
    "scriptPubKey": "76a914...",
    "amount": 25.50000000,
    "confirmations": 12
  }
]
```

**Use Case - UTXO Selection for Transactions:**
```python
def select_utxos_for_amount(rpc, target_amount, min_conf=6):
    """Select UTXOs to cover target amount"""
    utxos = rpc.call('listunspent', [min_conf])

    selected = []
    total = 0.0

    # Sort by amount (largest first) for efficiency
    utxos.sort(key=lambda x: x['amount'], reverse=True)

    for utxo in utxos:
        if total >= target_amount:
            break
        selected.append(utxo)
        total += utxo['amount']

    if total < target_amount:
        raise Exception(f"Insufficient funds: {total} < {target_amount}")

    return {
        'selected_utxos': selected,
        'total_input': total,
        'change': total - target_amount
    }

def get_spendable_balance(rpc, min_conf=6):
    """Get total spendable balance from UTXOs"""
    utxos = rpc.call('listunspent', [min_conf])
    return sum(utxo['amount'] for utxo in utxos)
```

---

## Transaction Operations

### getrawtransaction

Returns raw transaction data. Can return hex-encoded or decoded verbose format.

**Parameters:**
- `txid` (string, required) - Transaction ID
- `verbose` (number, optional, default=0) - If 0, returns hex string. If non-zero, returns decoded object.

**Returns:**
- If verbose=0: String (hex-encoded serialized transaction)
- If verbose!=0: Object containing full transaction details

**Example:**
```bash
./bitokd getrawtransaction "a1b2c3d4e5f6789012345678901234567890123456789012345678901234abcd"
./bitokd getrawtransaction "a1b2c3d4e5f6789012345678901234567890123456789012345678901234abcd" 1
```

**Response (verbose=0):**
```
"0100000001abcd..."
```

**Response (verbose=1):**
```json
{
  "hex": "0100000001abcd...",
  "txid": "a1b2c3d4e5f6789012345678901234567890123456789012345678901234abcd",
  "version": 1,
  "locktime": 0,
  "vin": [
    {
      "txid": "previous_txid_hash",
      "vout": 0,
      "scriptSig": "483045...",
      "sequence": 4294967295
    }
  ],
  "vout": [
    {
      "value": 50.0,
      "n": 0,
      "scriptPubKey": "76a914...",
      "address": "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"
    }
  ]
}
```

**Use Case - Block Explorer Transaction Details:**
```python
def get_full_transaction(rpc, txid):
    """Get complete transaction details for block explorer"""
    tx = rpc.call('getrawtransaction', [txid, 1])

    return {
        'txid': tx['txid'],
        'hex': tx['hex'],
        'inputs': tx['vin'],
        'outputs': tx['vout'],
        'total_output': sum(out['value'] for out in tx['vout'])
    }
```

---

### getrawmempool

Returns all transaction IDs currently in the memory pool (unconfirmed transactions).

**Parameters:** None

**Returns:** Array of transaction ID strings

**Example:**
```bash
./bitokd getrawmempool
```

**Response:**
```json
[
  "a1b2c3d4e5f6789012345678901234567890123456789012345678901234abcd",
  "b2c3d4e5f67890123456789012345678901234567890123456789012345bcde",
  "c3d4e5f678901234567890123456789012345678901234567890123456cdef"
]
```

**Use Case - Mempool Monitoring:**
```python
def monitor_mempool(rpc):
    """Monitor pending transactions in mempool"""
    mempool = rpc.call('getrawmempool')

    return {
        'pending_count': len(mempool),
        'transaction_ids': mempool
    }

def wait_for_confirmation(rpc, txid, timeout=600):
    """Wait for transaction to leave mempool (get confirmed)"""
    import time
    start = time.time()

    while time.time() - start < timeout:
        mempool = rpc.call('getrawmempool')
        if txid not in mempool:
            return True  # Transaction confirmed
        time.sleep(10)

    return False  # Still pending
```

---

### sendtoaddress

Sends BITOK to a specified address.

**Parameters:**
- `address` (string, required) - Destination Bitok address
- `amount` (number, required) - Amount to send (rounded to 0.01)
- `comment` (string, optional) - Comment for transaction (stored locally)
- `comment-to` (string, optional) - Comment about recipient (stored locally)

**Returns:** String (transaction ID / txid on success)

**Example:**
```bash
./bitokd sendtoaddress "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa" 10.50
./bitokd sendtoaddress "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa" 10.50 "payment" "Alice"
```

**Response:**
```
"a1b2c3d4e5f6789012345678901234567890123456789012345678901234abcd"
```

**Use Case - Exchange Withdrawal Processing:**
```python
def process_withdrawal(rpc, withdrawal_id, address, amount):
    """Process user withdrawal request"""

    # Validate address format
    if not validate_bitok_address(address):
        return {"error": "Invalid address"}

    # Check available balance
    balance = rpc.call('getbalance')
    if balance < amount:
        return {"error": "Insufficient balance"}

    # Send transaction
    try:
        comment = f"withdrawal_{withdrawal_id}"
        txid = rpc.call('sendtoaddress', [address, amount, comment])

        # Store txid for tracking
        db.execute(
            "UPDATE withdrawals SET status='sent', txid=?, sent_at=? WHERE id=?",
            (txid, datetime.now(), withdrawal_id)
        )

        return {"success": True, "txid": txid}
    except Exception as e:
        return {"error": str(e)}
```

**Important Notes:**
- Amount must be between 0.01 and 21,000,000 BITOK
- Amount is rounded to nearest 0.01 BITOK
- Transaction fee is automatically deducted from balance
- Comments are stored only in local wallet, not on blockchain

---

### listtransactions

Lists recent wallet transactions sorted by time (most recent first).

**Parameters:**
- `count` (number, optional, default=10) - Number of transactions to return
- `includegenerated` (boolean, optional, default=true) - Include generated (mined) transactions

**Returns:** Array of transaction objects containing:
- `txid` (string) - Transaction ID
- `category` (string) - "send", "receive", or "generate"
- `amount` (number) - Amount in BITOK (negative for sends)
- `fee` (number) - Transaction fee (only for sends)
- `address` (string) - Destination/source address
- `confirmations` (number) - Number of confirmations
- `time` (number) - Transaction timestamp

**Example:**
```bash
./bitokd listtransactions 20 true
```

**Response:**
```json
[
  {
    "txid": "a1b2c3d4e5f6789012345678901234567890123456789012345678901234abcd",
    "category": "receive",
    "amount": 50.00000000,
    "address": "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa",
    "confirmations": 145,
    "time": 1234567890
  },
  {
    "txid": "b2c3d4e5f67890123456789012345678901234567890123456789012345bcde",
    "category": "send",
    "amount": -10.00000000,
    "fee": -0.01000000,
    "address": "1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2",
    "confirmations": 12,
    "time": 1234567800
  },
  {
    "txid": "c3d4e5f678901234567890123456789012345678901234567890123456cdef",
    "category": "generate",
    "amount": 50.00000000,
    "confirmations": 200,
    "time": 1234567700
  }
]
```

**Use Case - Exchange Deposit Scanner:**
```python
def scan_recent_deposits(rpc, min_confirmations=6):
    """Scan recent transactions for confirmed deposits"""
    transactions = rpc.call('listtransactions', [100, False])

    deposits = []
    for tx in transactions:
        if tx['category'] == 'receive' and tx['confirmations'] >= min_confirmations:
            deposits.append({
                'txid': tx['txid'],
                'address': tx['address'],
                'amount': tx['amount'],
                'confirmations': tx['confirmations']
            })

    return deposits
```

---

## Mining Operations

### getgenerate

Returns the current mining status.

**Parameters:** None

**Returns:** Boolean (true if mining, false otherwise)

**Example:**
```bash
./bitokd getgenerate
```

**Response:**
```
true
```

---

### setgenerate

Enables or disables mining (generation of new blocks).

**Parameters:**
- `generate` (boolean, required) - Enable (true) or disable (false) mining
- `genproclimit` (number, optional) - Number of CPU threads to use (-1 = all cores)

**Returns:** null

**Example:**
```bash
./bitokd setgenerate true          # Enable mining with all cores
./bitokd setgenerate true 4        # Enable mining with 4 cores
./bitokd setgenerate false         # Disable mining
```

**Use Case - Mining Pool Worker Management:**
```python
def configure_mining(rpc, enable=True, threads=None):
    """Configure mining for pool worker"""

    if threads is None:
        # Use all available cores
        threads = -1

    rpc.call('setgenerate', [enable, threads])

    info = rpc.call('getinfo')
    return {
        'mining': info['generate'],
        'threads': info['genproclimit']
    }
```

**Performance Notes:**
- Uses Yespower proof-of-work algorithm (CPU-optimized)
- Automatically detects and uses SIMD instructions (SSE2/AVX/AVX2/AVX512)
- Memory-hard algorithm (~128KB RAM per hash)
- GPU/ASIC resistant by design

---

### getdifficulty

Returns the current proof-of-work difficulty.

**Parameters:** None

**Returns:** Number (difficulty value)

**Example:**
```bash
./bitokd getdifficulty
```

**Response:**
```
1.52587891
```

**Use Case - Mining Pool Difficulty Monitoring:**
```python
def monitor_network_difficulty(rpc):
    """Monitor difficulty for pool adjustment"""
    difficulty = rpc.call('getdifficulty')

    # Log for statistics
    log_difficulty(difficulty, datetime.now())

    # Adjust pool share difficulty
    pool_difficulty = difficulty * pool_difficulty_multiplier

    return {
        'network_difficulty': difficulty,
        'pool_difficulty': pool_difficulty
    }
```

---

## Network Operations

### getconnectioncount

Returns the number of connections to other nodes.

**Parameters:** None

**Returns:** Number (connection count)

**Example:**
```bash
./bitokd getconnectioncount
```

**Response:**
```
8
```

**Use Case - Node Health Monitoring:**
```python
def check_network_health(rpc):
    """Monitor peer connectivity"""
    connections = rpc.call('getconnectioncount')

    if connections == 0:
        alert("CRITICAL: Node isolated, no peer connections")
        return "critical"
    elif connections < 3:
        alert("WARNING: Low peer count")
        return "warning"
    else:
        return "healthy"
```

**Recommended:** Maintain at least 3-8 peer connections for reliable network participation.

---

### getpeerinfo

Returns data about each connected network node.

**Parameters:** None

**Returns:** Array of objects containing:
- `addr` (string) - IP address and port of the peer
- `services` (string) - Services offered by the peer (hex)
- `lastsend` (number) - Unix timestamp of last data sent
- `lastrecv` (number) - Unix timestamp of last data received
- `conntime` (number) - Unix timestamp when connection was established
- `version` (number) - Protocol version of the peer
- `inbound` (boolean) - True if peer initiated the connection
- `startingheight` (number) - Block height of peer when connected

**Example:**
```bash
./bitokd getpeerinfo
```

**Response:**
```json
[
  {
    "addr": "192.168.1.100:18333",
    "services": "00000001",
    "lastsend": 1609459200,
    "lastrecv": 1609459195,
    "conntime": 1609455600,
    "version": 209,
    "inbound": false,
    "startingheight": 12450
  },
  {
    "addr": "10.0.0.50:18333",
    "services": "00000001",
    "lastsend": 1609459198,
    "lastrecv": 1609459199,
    "conntime": 1609456000,
    "version": 209,
    "inbound": true,
    "startingheight": 12448
  }
]
```

**Use Case - Network Monitoring Dashboard:**
```python
def monitor_peers(rpc):
    """Get detailed peer information for monitoring"""
    peers = rpc.call('getpeerinfo')

    healthy_peers = 0
    for peer in peers:
        # Check if peer is active (sent/received within 5 minutes)
        now = time.time()
        if now - peer['lastrecv'] < 300:
            healthy_peers += 1

    return {
        'total_peers': len(peers),
        'healthy_peers': healthy_peers,
        'inbound': sum(1 for p in peers if p['inbound']),
        'outbound': sum(1 for p in peers if not p['inbound'])
    }
```

---

## Block Chain Operations

### getblockcount

Returns the total number of blocks in the longest block chain.

**Parameters:** None

**Returns:** Number (block count)

**Example:**
```bash
./bitokd getblockcount
```

**Response:**
```
12450
```

**Use Case - Blockchain Synchronization Check:**
```python
def check_sync_status(rpc, known_block_height):
    """Check if node is synchronized with network"""
    current_height = rpc.call('getblockcount')

    blocks_behind = known_block_height - current_height

    if blocks_behind > 100:
        print(f"WARNING: Node is {blocks_behind} blocks behind")
        return False

    return True
```

---

### getblocknumber

Returns the block number of the latest block (height of chain).

**Parameters:** None

**Returns:** Number (block number, 0-indexed)

**Example:**
```bash
./bitokd getblocknumber
```

**Response:**
```
12449
```

**Note:** `getblockcount` returns height + 1, while `getblocknumber` returns height (0-indexed).

---

### getbestblockhash

Returns the hash of the best (tip) block in the longest block chain.

**Parameters:** None

**Returns:** String (64-character hexadecimal block hash)

**Example:**
```bash
./bitokd getbestblockhash
```

**Response:**
```
"000007a5d9c7b6e7b8c9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1"
```

**Use Case - Quick Sync Check:**
```python
def check_chain_tip(rpc):
    """Get current chain tip for sync verification"""
    best_hash = rpc.call('getbestblockhash')
    block_count = rpc.call('getblockcount')

    return {
        'tip_hash': best_hash,
        'height': block_count
    }
```

---

### getblockhash

Returns the hash of the block at a specific height in the best blockchain.

**Parameters:**
- `index` (number, required) - Block height (0 = genesis block)

**Returns:** String (64-character hexadecimal block hash)

**Example:**
```bash
./bitokd getblockhash 0
```

**Response:**
```
"000007a5d9c7b6e7b8c9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1"
```

**Use Cases:**
- Block explorers navigating by height
- Verifying blockchain consistency
- Historical data queries

---

### getblock

Returns detailed information about a block given its hash.

**Parameters:**
- `hash` (string, required) - 64-character hexadecimal block hash

**Returns:** Object containing:
- `hash` (string) - Block hash
- `version` (number) - Block version
- `previousblockhash` (string) - Hash of previous block
- `merkleroot` (string) - Merkle root of transactions
- `time` (number) - Block timestamp (Unix epoch)
- `bits` (number) - Difficulty target in compact format
- `nonce` (number) - Nonce used to solve block
- `height` (number) - Block height in chain
- `tx` (array) - Array of transaction hashes
- `nextblockhash` (string, optional) - Hash of next block (if exists)

**Example:**
```bash
./bitokd getblock "000007a5d9c7b6e7b8c9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1"
```

**Response:**
```json
{
  "hash": "000007a5d9c7b6e7b8c9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1",
  "version": 1,
  "previousblockhash": "0000000000000000000000000000000000000000000000000000000000000000",
  "merkleroot": "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b",
  "time": 1231006505,
  "bits": 486604799,
  "nonce": 2083236893,
  "height": 0,
  "tx": [
    "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
  ]
}
```

**Use Cases:**
- Block explorers displaying block details
- Blockchain indexers processing blocks
- Verifying block contents

---

### gettransaction

Returns detailed information about a transaction. For wallet transactions, includes balance changes. For any transaction in the blockchain, returns raw transaction data.

**Parameters:**
- `txid` (string, required) - 64-character hexadecimal transaction ID

**Returns:** Object containing (for wallet transactions):
- `txid` (string) - Transaction ID
- `version` (number) - Transaction version
- `time` (number) - Time received
- `confirmations` (number) - Number of confirmations
- `blockhash` (string) - Block containing transaction
- `amount` (number) - Net amount (credit - debit)
- `fee` (number) - Transaction fee (for outgoing)
- `details` (array) - Array of transaction entries with:
  - `category` (string) - "send", "receive", or "generate"
  - `address` (string) - Address involved
  - `amount` (number) - Amount for this entry

For non-wallet transactions, returns:
- `txid` (string) - Transaction ID
- `version` (number) - Transaction version
- `vin` (array) - Input array with txid/vout or coinbase
- `vout` (array) - Output array with value, n, address

**Example:**
```bash
./bitokd gettransaction "a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2"
```

**Response (wallet transaction):**
```json
{
  "txid": "a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2",
  "version": 1,
  "time": 1609459200,
  "confirmations": 100,
  "blockhash": "000007a5d9c7b6e7b8c9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1",
  "amount": 50.0,
  "details": [
    {
      "category": "receive",
      "address": "1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2",
      "amount": 50.0
    }
  ]
}
```

**Use Cases:**
- Transaction lookup by ID
- Verifying payment receipt
- Exchange deposit/withdrawal tracking

---

### validateaddress

Validates a Bitok address and returns information about it.

**Parameters:**
- `address` (string, required) - Bitok address to validate

**Returns:** Object containing:
- `address` (string) - The address checked
- `isvalid` (boolean) - Whether the address is valid
- `ismine` (boolean) - Whether address belongs to wallet (only if valid)
- `label` (string) - Address label if in address book (only if valid)

**Example:**
```bash
./bitokd validateaddress "1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2"
```

**Response:**
```json
{
  "address": "1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2",
  "isvalid": true,
  "ismine": false
}
```

**Use Cases:**
- Validating user-provided addresses before sending
- Checking if address belongs to local wallet
- Address book management

---

## Integration Examples

### Exchange Deposit System

Complete example of automated deposit detection and crediting:

```python
import time
import sqlite3
from datetime import datetime

class ExchangeDepositSystem:
    def __init__(self, rpc):
        self.rpc = rpc
        self.db = sqlite3.connect('exchange.db')
        self.min_confirmations = 6

    def generate_deposit_address(self, user_id):
        """Generate unique deposit address for user"""
        label = f"user_{user_id}_deposit"
        address = self.rpc.call('getnewaddress', [label])

        # Store in database
        self.db.execute(
            "INSERT INTO deposit_addresses (user_id, address, created_at) "
            "VALUES (?, ?, ?)",
            (user_id, address, datetime.now())
        )
        self.db.commit()

        return address

    def scan_deposits(self):
        """Scan all addresses for new deposits"""
        addresses = self.db.execute(
            "SELECT id, user_id, address, last_amount FROM deposit_addresses"
        ).fetchall()

        for addr_id, user_id, address, last_amount in addresses:
            # Get current total received (1 confirmation minimum)
            current_amount = self.rpc.call('getreceivedbyaddress', [address, 1])

            if current_amount > last_amount:
                new_deposit = current_amount - last_amount

                # Check confirmations
                confirmed_amount = self.rpc.call(
                    'getreceivedbyaddress',
                    [address, self.min_confirmations]
                )

                if confirmed_amount >= current_amount:
                    # Fully confirmed - credit user
                    self.credit_user(user_id, new_deposit)

                    # Update last known amount
                    self.db.execute(
                        "UPDATE deposit_addresses SET last_amount=? WHERE id=?",
                        (current_amount, addr_id)
                    )
                    self.db.commit()

                    print(f"Credited {new_deposit} BITOK to user {user_id}")
                else:
                    # Pending confirmations
                    print(f"Pending: {new_deposit} BITOK for user {user_id}")

    def credit_user(self, user_id, amount):
        """Credit user account"""
        self.db.execute(
            "UPDATE users SET balance = balance + ? WHERE id = ?",
            (amount, user_id)
        )
        self.db.execute(
            "INSERT INTO transactions (user_id, type, amount, timestamp) "
            "VALUES (?, 'deposit', ?, ?)",
            (user_id, amount, datetime.now())
        )
        self.db.commit()

    def run(self):
        """Run deposit scanner continuously"""
        while True:
            try:
                self.scan_deposits()
            except Exception as e:
                print(f"Error scanning deposits: {e}")

            time.sleep(30)  # Check every 30 seconds

# Usage
rpc = BitokRPC('username', 'password')
deposit_system = ExchangeDepositSystem(rpc)
deposit_system.run()
```

---

### Exchange Withdrawal System

Complete example of automated withdrawal processing:

```python
class ExchangeWithdrawalSystem:
    def __init__(self, rpc):
        self.rpc = rpc
        self.db = sqlite3.connect('exchange.db')
        self.hot_wallet_minimum = 100.0  # BITOK

    def process_pending_withdrawals(self):
        """Process all pending withdrawal requests"""
        withdrawals = self.db.execute(
            "SELECT id, user_id, address, amount FROM withdrawals "
            "WHERE status='pending' ORDER BY created_at ASC"
        ).fetchall()

        for withdrawal_id, user_id, address, amount in withdrawals:
            self.process_withdrawal(withdrawal_id, user_id, address, amount)

    def process_withdrawal(self, withdrawal_id, user_id, address, amount):
        """Process single withdrawal"""
        try:
            # Validate address
            if not self.validate_address(address):
                self.mark_failed(withdrawal_id, "Invalid address")
                return

            # Check hot wallet balance
            balance = self.rpc.call('getbalance')
            if balance < amount:
                print(f"Insufficient hot wallet balance: {balance} < {amount}")
                return

            # Check hot wallet minimum
            if balance - amount < self.hot_wallet_minimum:
                print("Hot wallet would drop below minimum - refill needed")
                return

            # Send transaction
            comment = f"withdrawal_{withdrawal_id}"
            result = self.rpc.call('sendtoaddress', [address, amount, comment])

            # Mark as sent
            self.db.execute(
                "UPDATE withdrawals SET status='sent', sent_at=? WHERE id=?",
                (datetime.now(), withdrawal_id)
            )

            # Deduct from user balance
            self.db.execute(
                "UPDATE users SET balance = balance - ? WHERE id = ?",
                (amount, user_id)
            )

            self.db.commit()
            print(f"Processed withdrawal {withdrawal_id}: {amount} BITOK to {address}")

        except Exception as e:
            self.mark_failed(withdrawal_id, str(e))
            print(f"Withdrawal {withdrawal_id} failed: {e}")

    def validate_address(self, address):
        """Validate Bitok address format"""
        # Basic validation - should be enhanced
        return len(address) >= 26 and len(address) <= 35

    def mark_failed(self, withdrawal_id, reason):
        """Mark withdrawal as failed"""
        self.db.execute(
            "UPDATE withdrawals SET status='failed', error=? WHERE id=?",
            (reason, withdrawal_id)
        )
        self.db.commit()

    def run(self):
        """Run withdrawal processor continuously"""
        while True:
            try:
                self.process_pending_withdrawals()
            except Exception as e:
                print(f"Error processing withdrawals: {e}")

            time.sleep(10)  # Check every 10 seconds
```

---

### Block Explorer Transaction Tracker

Example of tracking and indexing blockchain transactions:

```python
class BlockExplorer:
    def __init__(self, rpc):
        self.rpc = rpc
        self.db = sqlite3.connect('explorer.db')

    def index_addresses(self):
        """Index all addresses with received amounts"""
        addresses = self.rpc.call('listreceivedbyaddress', [0, True])

        for addr_data in addresses:
            self.db.execute(
                "INSERT OR REPLACE INTO addresses "
                "(address, label, total_received, confirmations) "
                "VALUES (?, ?, ?, ?)",
                (
                    addr_data['address'],
                    addr_data['label'],
                    addr_data['amount'],
                    addr_data['confirmations']
                )
            )

        self.db.commit()
        print(f"Indexed {len(addresses)} addresses")

    def get_address_info(self, address):
        """Get comprehensive address information"""
        received = self.rpc.call('getreceivedbyaddress', [address, 0])
        label = self.rpc.call('getlabel', [address])

        return {
            'address': address,
            'label': label,
            'total_received': received,
            'balance': received  # Simplified - doesn't account for spent
        }

    def get_network_stats(self):
        """Get network statistics"""
        info = self.rpc.call('getinfo')
        block_count = self.rpc.call('getblockcount')

        return {
            'block_height': block_count,
            'difficulty': info['difficulty'],
            'connections': info['connections'],
            'mining': info['generate']
        }
```

---

### Mining Pool Work Distribution

Example of mining pool server integration:

```python
class MiningPoolServer:
    def __init__(self, rpc):
        self.rpc = rpc
        self.workers = {}

    def register_worker(self, worker_id, threads=4):
        """Register new mining worker"""
        self.workers[worker_id] = {
            'threads': threads,
            'shares': 0,
            'last_seen': datetime.now()
        }

        # Configure worker for mining
        self.rpc.call('setgenerate', [True, threads])

        return {
            'worker_id': worker_id,
            'difficulty': self.rpc.call('getdifficulty'),
            'block_height': self.rpc.call('getblockcount')
        }

    def update_worker_stats(self, worker_id, shares_submitted):
        """Update worker statistics"""
        if worker_id in self.workers:
            self.workers[worker_id]['shares'] += shares_submitted
            self.workers[worker_id]['last_seen'] = datetime.now()

    def get_pool_stats(self):
        """Get pool statistics"""
        info = self.rpc.call('getinfo')

        total_shares = sum(w['shares'] for w in self.workers.values())
        active_workers = sum(
            1 for w in self.workers.values()
            if (datetime.now() - w['last_seen']).seconds < 300
        )

        return {
            'difficulty': info['difficulty'],
            'block_height': info['blocks'],
            'active_workers': active_workers,
            'total_shares': total_shares,
            'pool_hashrate': self.estimate_hashrate()
        }

    def estimate_hashrate(self):
        """Estimate pool hashrate based on difficulty"""
        difficulty = self.rpc.call('getdifficulty')
        # Simplified calculation
        return difficulty * 2**32 / 600  # Assuming 10 min blocks
```

---

## Error Handling

### Common Error Codes

The RPC interface returns HTTP status codes and JSON-RPC errors:

#### HTTP Status Codes

- `200 OK` - Request successful
- `401 Unauthorized` - Invalid credentials
- `500 Internal Server Error` - RPC error occurred

#### RPC Error Object

```json
{
  "result": null,
  "error": {
    "code": -1,
    "message": "Error description"
  },
  "id": 1
}
```

### Common Errors

| Error | Cause | Solution |
|-------|-------|----------|
| Connection refused | Daemon not running | Start bitokd with `-server` |
| 401 Unauthorized | Wrong credentials | Check rpcuser/rpcpassword in config |
| Invalid address | Malformed address | Validate address format |
| Insufficient funds | Not enough balance | Check balance before sending |
| Invalid amount | Amount out of range | Use 0.01 to 21000000 |
| Wallet locked | Wallet encrypted | Unlock wallet (not in v0.3.0) |

### Error Handling Example

```python
def safe_rpc_call(rpc, method, params=[]):
    """Execute RPC call with error handling"""
    try:
        result = rpc.call(method, params)
        return {'success': True, 'result': result}

    except requests.exceptions.ConnectionError:
        return {'success': False, 'error': 'Cannot connect to Bitok daemon'}

    except requests.exceptions.HTTPError as e:
        if e.response.status_code == 401:
            return {'success': False, 'error': 'Authentication failed'}
        return {'success': False, 'error': f'HTTP error: {e}'}

    except Exception as e:
        # Parse JSON-RPC error
        try:
            error_data = e.response.json()
            error_msg = error_data.get('error', {}).get('message', str(e))
            return {'success': False, 'error': error_msg}
        except:
            return {'success': False, 'error': str(e)}

# Usage
result = safe_rpc_call(rpc, 'getbalance')
if result['success']:
    print(f"Balance: {result['result']}")
else:
    print(f"Error: {result['error']}")
```

---

## Security Best Practices

### 1. Credential Management

All settings are command-line only - there is no config file:

```bash
# Strong credentials (32+ characters recommended)
./bitokd -rpcuser=bitok_rpc_user_a8f7d9c2b1e4 \
         -rpcpassword=9k3mX7pQ2vL5wN8rT4yH6jU1cF9dE0aZ \
         -server -daemon
```

Or use a wrapper script so you don't have to type them every time:

```bash
#!/bin/bash
# /usr/local/bin/bitok-rpc
exec /path/to/bitokd \
    -rpcuser=bitok_rpc_user_a8f7d9c2b1e4 \
    -rpcpassword=9k3mX7pQ2vL5wN8rT4yH6jU1cF9dE0aZ \
    "$@"
```

### 2. Network Security

```bash
# Firewall rules (iptables example)
# Allow RPC only from specific IPs
iptables -A INPUT -p tcp --dport 8332 -s 10.0.1.0/24 -j ACCEPT
iptables -A INPUT -p tcp --dport 8332 -j DROP

# Or use SSH tunnel for remote access
ssh -L 8332:localhost:8332 user@bitok-server
```

RPC binds to 127.0.0.1 by default. Keep it that way.

### 3. Hot Wallet Management

**Best Practices:**
- Keep minimum funds in hot wallet
- Use cold storage for bulk holdings
- Implement automatic cold→hot transfers
- Monitor hot wallet balance continuously
- Set up alerts for unusual activity

```python
def manage_hot_wallet(rpc, cold_wallet_address):
    """Automatic hot wallet management"""
    hot_balance = rpc.call('getbalance')

    # Refill if below minimum
    if hot_balance < HOT_WALLET_MINIMUM:
        needed = HOT_WALLET_TARGET - hot_balance
        print(f"Hot wallet needs refill: {needed} BITOK")
        # Manual cold wallet transfer required
        send_refill_alert(needed)

    # Sweep excess to cold storage
    elif hot_balance > HOT_WALLET_MAXIMUM:
        excess = hot_balance - HOT_WALLET_TARGET
        print(f"Sweeping {excess} BITOK to cold storage")
        rpc.call('sendtoaddress', [cold_wallet_address, excess, "cold_storage"])
```

### 4. Transaction Verification

Always verify critical transactions:

```python
def verify_transaction(rpc, address, expected_amount, min_conf=6):
    """Verify transaction with multiple checks"""

    # Check received amount
    received = rpc.call('getreceivedbyaddress', [address, min_conf])

    if received < expected_amount:
        return False, f"Insufficient amount: {received} < {expected_amount}"

    # Check confirmations
    received_1conf = rpc.call('getreceivedbyaddress', [address, 1])
    received_6conf = rpc.call('getreceivedbyaddress', [address, 6])

    if received_1conf > received_6conf:
        return False, "Transaction not fully confirmed"

    return True, "Verified"
```

### 5. Rate Limiting

Implement rate limiting to prevent abuse:

```python
from collections import defaultdict
import time

class RateLimiter:
    def __init__(self, max_requests=100, window=60):
        self.max_requests = max_requests
        self.window = window  # seconds
        self.requests = defaultdict(list)

    def allow_request(self, client_id):
        """Check if request is allowed"""
        now = time.time()

        # Clean old requests
        self.requests[client_id] = [
            req_time for req_time in self.requests[client_id]
            if now - req_time < self.window
        ]

        # Check limit
        if len(self.requests[client_id]) >= self.max_requests:
            return False

        self.requests[client_id].append(now)
        return True

# Usage
limiter = RateLimiter(max_requests=100, window=60)

def rate_limited_rpc_call(rpc, client_id, method, params):
    if not limiter.allow_request(client_id):
        raise Exception("Rate limit exceeded")

    return rpc.call(method, params)
```

### 6. Audit Logging

Log all critical operations:

```python
import logging

logging.basicConfig(
    filename='bitok_rpc.log',
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

def log_transaction(action, details):
    """Log transaction for audit trail"""
    logging.info(f"{action}: {json.dumps(details)}")

# Usage
def send_with_logging(rpc, address, amount, user_id):
    log_transaction('withdrawal_initiated', {
        'user_id': user_id,
        'address': address,
        'amount': amount
    })

    try:
        result = rpc.call('sendtoaddress', [address, amount])
        log_transaction('withdrawal_success', {
            'user_id': user_id,
            'address': address,
            'amount': amount
        })
        return result
    except Exception as e:
        log_transaction('withdrawal_failed', {
            'user_id': user_id,
            'address': address,
            'amount': amount,
            'error': str(e)
        })
        raise
```

---

## Appendix

### Complete RPC Command Reference

| Command | Category | Description |
|---------|----------|-------------|
| help | General | List all commands |
| stop | General | Stop server |
| getinfo | General | Get server information |
| getblockcount | Blockchain | Get total block count |
| getblocknumber | Blockchain | Get current block height |
| getbestblockhash | Blockchain | Get hash of best (tip) block |
| getblockhash | Blockchain | Get block hash by height |
| getblock | Blockchain | Get block data by hash |
| gettransaction | Transaction | Get transaction details by txid |
| getrawtransaction | Transaction | Get raw transaction data (hex or decoded) |
| getrawmempool | Transaction | Get all unconfirmed transaction IDs |
| validateaddress | Utility | Validate address and check ownership |
| getdifficulty | Mining | Get proof-of-work difficulty |
| getconnectioncount | Network | Get peer count |
| getpeerinfo | Network | Get detailed peer information |
| getgenerate | Mining | Get mining status |
| setgenerate | Mining | Enable/disable mining |
| getbalance | Wallet | Get wallet balance |
| getnewaddress | Wallet | Generate new address |
| setlabel | Wallet | Set address label |
| getlabel | Wallet | Get address label |
| getaddressesbylabel | Wallet | Get addresses by label |
| sendtoaddress | Transaction | Send coins |
| listtransactions | Transaction | List recent transactions |
| listunspent | Wallet | List unspent transaction outputs (UTXOs) |
| getreceivedbyaddress | Wallet | Get amount received by address |
| getreceivedbylabel | Wallet | Get amount received by label |
| listreceivedbyaddress | Wallet | List received by address |
| listreceivedbylabel | Wallet | List received by label |

### Version History

- **Bitcoin v0.3.19** (2010) - Latest original implementation by Satoshi Nakamoto
- **Bitok** (2016) - Modern system compatibility, Yespower integration

### Additional Resources

- Bitok Repository: https://github.com/elvisjedusor/bitok
- Bitcoin Original Paper: https://bitcoin.org/bitcoin.pdf
- Yespower Algorithm: https://www.openwall.com/yespower/
- JSON-RPC Specification: https://www.jsonrpc.org/specification_v1

### Support and Community

For technical support and integration assistance:

- GitHub Issues: https://github.com/elvisjedusor/bitok/issues
- Review source code: `rpc.cpp` for implementation details

---

**Document Version:** 1.0
**Last Updated:** 2026
**License:** MIT/X11

*This documentation covers the RPC API as implemented in Bitok 0.3.19 Mainnet. Always test thoroughly in a development environment before deploying to production.*

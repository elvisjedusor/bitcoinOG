# Bitok - macOS Build Instructions

**Bitcoin v0.3.19 with GPU-resistant mining - Modern macOS build**

This is Satoshi's original code from 2010, adapted to run on modern macOS with CPU-friendly proof-of-work.

For mining details, see [BITOKPOW.md](BITOKPOW.md).
For philosophy, see [MANIFESTO.md](MANIFESTO.md).

---

Modern build instructions for macOS 11.0+ (Big Sur and later)
Supports both Apple Silicon (arm64) and Intel (x86_64)

## Prerequisites

### 1. Install Xcode Command Line Tools

```bash
xcode-select --install
```

### 2. Install Homebrew

If you don't have Homebrew installed:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 3. Install Dependencies

```bash
brew install boost berkeley-db@4 openssl@3 wxwidgets
```

## Building

### Build the Daemon (bitokd)

```bash
make -f makefile.osx bitokd
```

### Build the GUI Wallet (bitok)

```bash
make -f makefile.osx bitok
```

### Build Both

```bash
make -f makefile.osx all
```

## CPU Optimization

Default builds create universal binaries (ARM + Intel). For native builds optimized for your CPU:

```bash
make -f makefile.osx ARCH="$(uname -m)" all  # Native build with -march=native
```

**For distribution binaries:**
```bash
make -f makefile.osx all                           # Universal binary (default)
make -f makefile.osx ARCH=x86_64 YESPOWER_ARCH=x86-64-v3 all  # Intel optimized
make -f makefile.osx ARCH=arm64 all                # Apple Silicon only
```

## Installation

Install daemon to /usr/local/bin:

```bash
sudo make -f makefile.osx install
```

Install GUI wallet to /Applications:

```bash
sudo make -f makefile.osx install-gui
```

## Running

### Run the Daemon

```bash
./bitokd
```

### Run the GUI Wallet

```bash
./bitok
```

Or after installation:

```bash
open /Applications/Bitok.app
```

## Troubleshooting

### Architecture Issues

The makefile builds universal binaries (arm64 + x86_64). If you need to build for a specific architecture:

```bash
# Apple Silicon only
make -f makefile.osx ARCH=arm64

# Intel only
make -f makefile.osx ARCH=x86_64
```

### Dependency Path Issues

If Homebrew installed dependencies in a non-standard location, you may need to set:

```bash
export HOMEBREW_PREFIX=$(brew --prefix)
make -f makefile.osx
```

### OpenSSL 3.x Compatibility

The code includes OpenSSL 3.x compatibility fixes. If you encounter OpenSSL-related errors, ensure you have OpenSSL 3.x installed:

```bash
brew info openssl@3
```

### wxWidgets Issues

Ensure wxWidgets 3.2+ is installed with proper configuration:

```bash
wx-config --version  # Should show 3.2 or higher
wx-config --libs     # Should show library paths
```

## Clean Build

To remove all compiled objects and binaries:

```bash
make -f makefile.osx clean
```

## Notes

- Data directory: `~/Library/Application Support/Bitok/`
- Wallet file: `~/Library/Application Support/Bitok/wallet.dat`

All settings are passed via command line arguments - there is no configuration file.

```bash
./bitokd -daemon                           # Run in background
./bitokd -gen                              # Enable mining
./bitokd -rpcuser=user -rpcpassword=pass   # Set RPC credentials
./bitokd -addnode=192.168.1.100            # Connect to specific node
./bitokd -?                                # Show all options
```

**IMPORTANT**: Back up your wallet.dat file regularly!

## Building for Distribution

To create a distributable .app bundle:

```bash
make -f makefile.osx bundle
```

This creates `Bitok.app` that can be copied to /Applications or distributed to other Macs.

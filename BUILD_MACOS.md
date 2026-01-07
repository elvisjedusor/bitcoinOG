# Bitok - macOS Build Instructions

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
- Configuration file: `~/Library/Application Support/Bitok/bitok.conf`
- Wallet file: `~/Library/Application Support/Bitok/wallet.dat`

**IMPORTANT**: Back up your wallet.dat file regularly!

## Building for Distribution

To create a distributable .app bundle:

```bash
make -f makefile.osx bundle
```

This creates `Bitok.app` that can be copied to /Applications or distributed to other Macs.

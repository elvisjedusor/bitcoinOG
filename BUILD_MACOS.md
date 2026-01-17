# Building Bitok on macOS

Supports macOS 11.0+ (Big Sur and later), Apple Silicon (arm64) and Intel (x86_64).

## Prerequisites

```bash
xcode-select --install
brew install boost berkeley-db@4 openssl@3 wxwidgets
```

## Building

```bash
# GUI wallet
make -f makefile.osx bitok

# Daemon only
make -f makefile.osx bitokd

# Both
make -f makefile.osx all

# Clean
make -f makefile.osx clean
```

## CPU Optimization

Default build uses native architecture with `-march=native` for best performance.

```bash
# Explicit architecture
make -f makefile.osx ARCH=arm64 all      # Apple Silicon
make -f makefile.osx ARCH=x86_64 all     # Intel

# Intel with AVX2 optimizations
make -f makefile.osx ARCH=x86_64 YESPOWER_ARCH=x86-64-v3 all
```

## Installation

```bash
# Daemon to /usr/local/bin
sudo make -f makefile.osx install

# GUI binary to /usr/local/bin
sudo make -f makefile.osx install-gui

# GUI .app bundle to /Applications
make -f makefile.osx bundle
sudo cp -r Bitok.app /Applications/
```

## Running

Data directory: `~/Library/Application Support/Bitok/`

```bash
./bitokd                           # Daemon
./bitokd -gen                      # Mine
./bitokd -daemon                   # Background
./bitokd -server -rpcuser=u -rpcpassword=p

./bitok                            # GUI wallet
open /Applications/Bitok.app       # After install-gui
```

## Troubleshooting

### Dependency paths

```bash
export HOMEBREW_PREFIX=$(brew --prefix)
make -f makefile.osx
```

### wxWidgets

```bash
wx-config --version   # Should show 3.2+
wx-config --libs
```

### OpenSSL

```bash
brew info openssl@3
```

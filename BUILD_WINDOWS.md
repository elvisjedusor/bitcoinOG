# Bitok - Windows Build Instructions

Modern build instructions for Windows 10+ (64-bit)

## Two Build Methods

### Method 1: Cross-Compile from Ubuntu (Recommended)

This allows you to build Windows binaries from your Ubuntu machine.

#### Prerequisites on Ubuntu

```bash
sudo apt-get update
sudo apt-get install -y \
    mingw-w64 \
    g++-mingw-w64-x86-64 \
    wine64
```

#### Install Windows Dependencies (via Ubuntu)

You'll need to build or download Windows versions of dependencies. The easiest way is using pre-built packages:

```bash
# Install MXE (M cross environment) for easier cross-compilation
git clone https://github.com/mxe/mxe.git
cd mxe
make MXE_TARGETS='x86_64-w64-mingw32.static' boost wxwidgets openssl
```

Alternatively, use the provided makefile with manual dependency setup (see below).

#### Build from Ubuntu

```bash
make -f makefile.mingw bitokd.exe  # Daemon only
make -f makefile.mingw bitok.exe   # GUI wallet
make -f makefile.mingw all         # Both
```

#### Test on Ubuntu (using Wine)

```bash
wine64 bitokd.exe --help
```

---

### Method 2: Native Windows Build (MSYS2)

Build directly on Windows 10+ using MSYS2.

#### Prerequisites on Windows

1. Download and install MSYS2 from: https://www.msys2.org/

2. Open MSYS2 MinGW 64-bit terminal and install dependencies:

```bash
pacman -Syu
pacman -S --needed \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-boost \
    mingw-w64-x86_64-openssl \
    mingw-w64-x86_64-db \
    mingw-w64-x86_64-wxWidgets3.2 \
    make
```

#### Build on Windows

```bash
make -f makefile.mingw bitokd.exe  # Daemon only
make -f makefile.mingw bitok.exe   # GUI wallet
make -f makefile.mingw all         # Both
```

---

## Running on Windows

### GUI Wallet

Double-click `bitok.exe` or run from command prompt:

```cmd
bitok.exe
```

### Daemon

```cmd
bitokd.exe
```

Or with arguments:

```cmd
bitokd.exe -daemon
bitokd.exe -server -rpcuser=user -rpcpassword=pass
```

---

## Configuration

Data directory: `%APPDATA%\Bitok\`

Create configuration file: `%APPDATA%\Bitok\bitok.conf`

Example:

```
server=1
rpcuser=yourusername
rpcpassword=yourpassword
rpcport=8332
```

---

## Building with MSVC (Alternative)

If you prefer Microsoft Visual Studio:

### Prerequisites

1. Install Visual Studio 2019 or later with C++ desktop development
2. Install vcpkg for dependencies:

```cmd
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
vcpkg integrate install
vcpkg install boost-system:x64-windows boost-filesystem:x64-windows openssl:x64-windows berkeleydb:x64-windows wxwidgets:x64-windows
```

### Build with MSVC

```cmd
nmake -f makefile.vc
```

---

## Troubleshooting

### Missing DLLs

If you get DLL errors when running, you may need to:

1. **Copy MinGW DLLs** to the same directory as the .exe:
   - libgcc_s_seh-1.dll
   - libstdc++-6.dll
   - libwinpthread-1.dll

2. **Or build static**: The makefile is configured for static linking by default

### OpenSSL 3.x Compatibility

The code includes OpenSSL 3.x compatibility. Ensure you're using OpenSSL 3.0+.

### 32-bit vs 64-bit

The modern build targets 64-bit only. For 32-bit builds, modify makefile.mingw:

```makefile
CROSS_PREFIX = i686-w64-mingw32-
```

---

## Cross-Compilation Details

When cross-compiling from Ubuntu, the makefile automatically detects MinGW-w64 and uses:

- Compiler: `x86_64-w64-mingw32-g++`
- Target: Windows 10+ (64-bit)
- Static linking for portability

The resulting .exe files can be copied directly to Windows machines.

---

## Clean Build

```bash
make -f makefile.mingw clean
```

---

## Creating Installer (Optional)

Use NSIS (Nullsoft Scriptable Install System):

1. Install NSIS from: https://nsis.sourceforge.io/
2. Edit `setup.nsi` with correct paths
3. Right-click `setup.nsi` → "Compile NSIS Script"

This creates `BitokSetup.exe` installer.

---

## Notes

**IMPORTANT**:
- Back up your wallet.dat file regularly
- The wallet is stored in `%APPDATA%\Bitok\wallet.dat`
- Never share your wallet.dat or private keys
- Test with small amounts first

**Security**:
- Windows Defender may flag cryptocurrency software
- Add exception if needed: Settings → Virus & threat protection → Exclusions
- Always verify checksums of downloaded binaries

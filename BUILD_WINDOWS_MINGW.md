# Building Bitok on Windows with MinGW

## Prerequisites

### Option A: MSYS2 (Native Windows)

1. Download and install MSYS2 from https://www.msys2.org/
2. Open **MSYS2 MinGW 64-bit** terminal
3. Install dependencies:

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

### Option B: Cross-Compile from Ubuntu

```bash
sudo apt-get install mingw-w64 g++-mingw-w64-x86-64

# For dependencies, use MXE:
git clone https://github.com/mxe/mxe.git
cd mxe
make MXE_TARGETS='x86_64-w64-mingw32.static' boost wxwidgets openssl
```

## Building

```bash
# GUI wallet
make -f makefile.mingw gui

# Daemon only
make -f makefile.mingw daemon

# Both
make -f makefile.mingw all

# Clean
make -f makefile.mingw clean
```

## CPU Optimization

```bash
# For your specific CPU (best performance)
make -f makefile.mingw all MARCH=native

# For distribution (modern CPUs, 2015+)
make -f makefile.mingw all MARCH=x86-64-v3

# Maximum compatibility
make -f makefile.mingw all MARCH=x86-64
```

## Output

- `bitok.exe` - GUI wallet
- `bitokd.exe` - Command-line daemon

## Running

Data directory: `%APPDATA%\Bitok\`

```cmd
bitok.exe                           # GUI wallet
bitokd.exe                          # Daemon
bitokd.exe -daemon                  # Background
```

Configure settings in `%APPDATA%\Bitok\bitok.conf`. See [README.md](README.md) for config options.

## Troubleshooting

### Berkeley DB Compatibility

**Note:** Different build methods use different BDB versions:

- MSYS2 `mingw-w64-x86_64-db` installs BDB 5.3+
- Ubuntu `libdb5.3-dev` installs BDB 5.3
- MSVC/vcpkg uses BDB 4.8.30 (vcpkg only provides 4.8)

MinGW and Ubuntu builds share wallet format. MSVC builds have different wallet format.

If migrating wallet between BDB versions, use: `bitokd -recover`

### Missing DLLs

Build is configured for static linking. If you get DLL errors, copy these from MSYS2:
- libgcc_s_seh-1.dll
- libstdc++-6.dll
- libwinpthread-1.dll

### Windows Defender

May flag cryptocurrency software. Add exclusion in Settings if needed.

# Building Bitok on Windows with Visual Studio

## Prerequisites

### 1. Visual Studio 2019+

Install with "Desktop development with C++" workload.

### 2. vcpkg

```cmd
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
bootstrap-vcpkg.bat
vcpkg integrate install
```

### 3. Dependencies

For daemon only:
```cmd
vcpkg install boost:x64-windows-static berkeleydb:x64-windows-static openssl:x64-windows-static zlib:x64-windows-static
```

For GUI (includes all daemon deps plus wxWidgets and its dependencies):
```cmd
vcpkg install boost:x64-windows-static berkeleydb:x64-windows-static openssl:x64-windows-static zlib:x64-windows-static expat:x64-windows-static liblzma:x64-windows-static libpng:x64-windows-static libjpeg-turbo:x64-windows-static tiff:x64-windows-static wxwidgets:x64-windows-static
```

**Note:** vcpkg wxWidgets uses system libraries (expat, liblzma, libpng, libjpeg-turbo, tiff) instead of bundled versions. These must be installed explicitly.

## Building

Open **x64 Native Tools Command Prompt for VS** from Start menu.

```cmd
set VCPKG_ROOT=C:\vcpkg

# Verify setup
nmake -f makefile.vc check

# Build daemon
nmake -f makefile.vc daemon

# Build GUI
nmake -f makefile.vc gui

# Build both
nmake -f makefile.vc both

# Clean
nmake -f makefile.vc clean

# Help
nmake -f makefile.vc help
```

## Output

- `bitok.exe` - GUI wallet
- `bitokd.exe` - Command-line daemon

## Troubleshooting

### Run `check` first

```cmd
nmake -f makefile.vc check
```

This verifies vcpkg setup and shows which dependencies are missing.

### Berkeley DB version

vcpkg may install libdb48.lib or libdb53.lib. Check and update makefile.vc if needed:

```cmd
dir C:\vcpkg\installed\x64-windows-static\lib\libdb*.lib
```

Edit `DB_LIB=libdb48.lib` in makefile.vc to match.

### wxWidgets version

Check actual version and update `WX_VER` in makefile.vc:

```cmd
dir C:\vcpkg\installed\x64-windows-static\lib\wx*.lib
```

- `wxmsw32u_*` = WX_VER=32 (wxWidgets 3.2)
- `wxmsw33u_*` = WX_VER=33 (wxWidgets 3.3)

### Linker errors

1. Verify all packages installed for `x64-windows-static` triplet
2. Check library names match vcpkg installation
3. Run from **x64 Native Tools Command Prompt** (not regular cmd)

## Running

Data directory: `%APPDATA%\Bitok\`

```cmd
bitok.exe                           # GUI wallet
bitokd.exe                          # Daemon
bitokd.exe -gen                     # Mine
bitokd.exe -server -rpcuser=u -rpcpassword=p
```

# Changelog

All notable changes to Bitok are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.19] - 2026-01-10 (Bitok Mainnet) **MAJOR**

### Added
- GPU-resistant Yespower proof-of-work algorithm
- Modern C++11 codebase compatible with current compilers
- Cross-platform build system (Linux, Windows, macOS)
- SHA-NI hardware acceleration for supported CPUs
- Static linking for portable Windows binaries
- Configuration file support (bitok.conf)
- Mining RPC commands for pool integration:
  - `getmininginfo` - Mining statistics
  - `getblocktemplate` - BIP 22 block template
  - `submitblock` - Block submission
  - `getwork` - Legacy mining protocol

### Changed
- Updated OpenSSL to 3.x compatibility
- Modernized Berkeley DB interface
- Improved build system with auto-detection

### Security
- Removed deprecated cryptographic functions
- Added secure memory zeroing for sensitive data

### Fixed
- Compilation errors on modern GCC/Clang
- Windows 10+ compatibility issues

---

## [v0.3.19] - 2010-12-13 (Latest Satoshi Release)

Original Bitcoin v0.3.19 by Satoshi Nakamoto.

This version serves as the historical baseline for Bitok.

---

## Versioning Policy

- **MAJOR**: Consensus-breaking changes (hard forks)
- **MINOR**: New features, soft forks, protocol upgrades
- **PATCH**: Bug fixes, security patches, minor improvements

Pre-release versions use suffixes:
- `-alpha.N` - Early development, unstable
- `-beta.N` - Feature complete, testing phase
- `-rc.N` - Release candidate, final testing

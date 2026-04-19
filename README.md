# PingTool

Multi-host ICMP ping utility for Windows with native Win32 GUI.

## Features

- Multi-host ICMP ping with IcmpSendEcho2
- Support for 1000+ concurrent targets
- CIDR notation support (e.g., 192.168.1.0/24)
- Native Win32 ListView GUI (dark theme)
- CSV/TXT/Excel import/export
- Auto IP column detection
- Configurable export columns
- JSON config persistence
- GitHub Actions CI/CD

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Usage

1. Enter IP addresses, hostnames, or CIDR notation
2. Click "Add" to add hosts to the list
3. Click "Start" to begin ping
4. Results are updated in real-time
5. Click column headers to sort

## License

MIT
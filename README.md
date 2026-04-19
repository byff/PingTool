# PingTool

Cross-platform multi-target concurrent Ping tool with modern dark UI.

## Features

- Multi-target concurrent Ping (1000+ IPs supported)
- Support IP, CIDR, domain input
- Real-time result table with sortable columns
- Import IP list from TXT/Excel with drag-and-drop
- Auto-detect IP columns in Excel
- Export results to Excel / append to source file
- Configurable timeout, packet size, interval, concurrency
- Local config persistence
- Modern dark tech-style GUI

## Tech Stack

- **Rust** - Memory safety, high performance
- **Tauri 2.x** - Embedded WebView2, single file ~10MB
- **React + Vite** - Modern frontend
- **calamine** - Pure Rust Excel library

## Build

```bash
# Install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# Install Node.js dependencies
npm install

# Development
npm run tauri dev

# Production build
npm run tauri build
```

## Build Targets

| Platform | Output |
|----------|--------|
| Windows | `PingTool.exe` (~10MB) |
| Linux x64 | `PingTool` (~12MB) |
| Linux ARM64 | `PingTool` (~12MB) |

## Config

Config file location:
- Windows: `%APPDATA%\PingTool\config.json`
- Linux: `~/.config/PingTool/config.json`

## License

MIT

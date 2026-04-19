# PingTool

高性能多目标 Ping 工具，支持同时 Ping 1000+ 个 IP。

## 功能

- 多目标并发 Ping（支持 1000+ IP）
- 支持 IP、CIDR、域名输入
- 实时结果表格，所有列支持排序
- 从 TXT / Excel 导入 IP 列表，支持拖拽
- Excel 自动识别 IP 列
- 导出结果到 Excel / 插入结果到源表
- 可配置超时、包大小、间隔、并发数
- 配置文件本地持久化
- 现代深色科技风 GUI

## 平台支持

| 平台 | 架构 | 状态 |
|------|------|------|
| Windows | x86_64 | ✅ |
| Linux (麒麟/Kylin) | aarch64 | ✅ |
| Linux | x86_64 | ✅ |

## 下载

从 [Releases](https://github.com/byff/PingTool/releases) 下载预编译版本。

## 编译

### 前置依赖

- Go 1.21+
- Node.js 20+
- Wails CLI

### 编译步骤

```bash
# 安装 Wails CLI
go install github.com/wailsapp/wails/v2/cmd/wails@latest

# 克隆项目
git clone https://github.com/byff/PingTool.git
cd PingTool

# 下载依赖
go mod tidy
cd frontend && npm install && cd ..

# 运行开发模式
wails dev

# 打包发布
wails build
```

## 技术栈

- Go + Wails 2.x（跨平台桌面应用）
- Vue 3 + TypeScript（前端 UI）
- ant-design-vue（组件库）
- excelize（Excel 处理）

## License

MIT

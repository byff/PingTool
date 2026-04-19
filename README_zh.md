# PingTool

高性能多目标Ping工具，支持同时Ping 1000+个IP。

## 功能特点

- ✅ 多目标并发Ping（支持1000+ IP）
- ✅ 支持IP、CIDR、域名输入
- ✅ 实时结果表格，所有列支持排序
- ✅ 从TXT/Excel导入IP列表，支持拖拽
- ✅ Excel自动识别IP列
- ✅ 导出结果到Excel/插入结果到源表
- ✅ 可配置超时、包大小、间隔、并发数
- ✅ 配置文件本地持久化
- ✅ 现代深色科技风GUI

## 平台支持

| 平台 | 架构 | 状态 |
|------|------|------|
| Windows | x86_64 | ✅ |
| Linux (麒麟/Kylin) | aarch64 | ✅ |
| Linux | x86_64 | ✅ |

## 快速开始

### 使用预编译版本

从 [Releases](https://github.com/byff/PingTool/releases) 下载对应平台的可执行文件。

### 从源码编译

```bash
# 1. 安装依赖
# - Go 1.21+
# - Node.js 20+
# - Wails CLI: go install github.com/wailsapp/wails/v2/cmd/wails@latest

# 2. 克隆项目
git clone https://github.com/byff/PingTool.git
cd PingTool

# 3. 安装Go依赖
go mod tidy

# 4. 安装前端依赖
cd frontend
npm install
cd ..

# 5. 运行开发模式
wails dev

# 6. 打包发布
wails build
```

## 技术栈

- **后端**: Go + Wails 2.x
- **前端**: Vue 3 + TypeScript + ant-design-vue
- **Excel处理**: excelize

## License

MIT License

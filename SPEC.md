# PingTool - 跨平台多目标并发Ping工具

## 1. 项目概述

**项目名称**: PingTool
**项目类型**: 跨平台桌面工具 (Windows / Linux ARM)
**核心功能**: 支持 1000+ IP 的并发 Ping 探测，提供实时可视化结果
**目标用户**: 网络运维工程师、系统管理员、SRE

---

## 2. 技术栈

| 层级 | 技术选型 | 说明 |
|------|----------|------|
| 语言 | Rust 1.75+ | 内存安全、高性能、无 GC |
| GUI 框架 | Tauri 2.x | 内置 WebView2，单文件 ~10MB |
| 前端 | HTML/CSS/JS (Vite) | 现代化前端构建 |
| Excel 处理 | calamine | 纯 Rust Excel 库，无 C 依赖 |
| 并发 | tokio | 异步运行时，高效并发 |
| 跨平台编译 | `cargo build --target x86_64-pc-windows-msvc`<br>`cargo build --target aarch64-unknown-linux-gnu` | 一套代码多平台 |

### 为什么选择 Rust + Tauri？

| 对比 | Python | Go + Fyne | **Rust + Tauri** |
|------|--------|-----------|------------------|
| 打包体积 | 50-100MB | ~35MB | **5-15MB** |
| 启动速度 | 2-5秒 | 1-2秒 | **<100ms** |
| 内存安全 | 无保证 | 无保证 | **编译期保证** |
| 依赖 | 需 Python | 无外部依赖 | **无运行时依赖** |
| ARM 支持 | 需编译版 | 原生 | **原生** |
| Windows 兼容性 | 良好 | 良好 | **WebView2 (Win10+预装)** |

---

## 3. 功能规格

### 3.1 IP 输入

| 功能 | 描述 |
|------|------|
| 单IP输入 | 手动输入单个 IP 地址 |
| IP 范围 | 支持 CIDR 格式 (e.g., `192.168.1.0/24`) |
| 域名输入 | 支持域名解析后 Ping |
| 批量输入 | 换行分隔多个 IP/域名 |
| 输入验证 | 非法格式即时提示 |

### 3.2 文件导入

| 功能 | 描述 |
|------|------|
| TXT 导入 | 每行一个 IP，支持拖拽 |
| Excel 导入 | 支持 .xlsx/.xls，自动识别 IP 列 |
| IP 列识别 | 通过正则表达式匹配 IP 格式 |
| 拖拽支持 | 拖拽文件到窗口导入 |
| 导入预览 | 导入前显示检测到的 IP 列表 |

### 3.3 并发 Ping 引擎

| 参数 | 默认值 | 可调范围 |
|------|--------|----------|
| 超时时间 | 2000ms | 100-10000ms |
| 包大小 | 64 bytes | 32-65535 bytes |
| Ping 间隔 | 0ms (连续) | 0-10000ms |
| 并发数 | 100 | 1-1000 |
| Ping 次数 | 4 次/目标 | 1-100 次 |

### 3.4 结果展示

| 功能 | 描述 |
|------|------|
| 实时表格 | 显示所有探测结果 |
| 可排序列 | IP/域名、状态、延迟(avg/min/max)、TTL、丢包率 |
| 状态图标 | 在线(绿)、超时(红)、解析失败(灰) |
| 统计摘要 | 在线数/离线数/总完成数 |
| 进度指示 | 整体进度条 + 当前批次 |

### 3.5 导出功能

| 功能 | 描述 |
|------|------|
| 导出 Excel | 新建 xlsx 文件，包含完整结果 |
| 回写源表 | 追加结果列到原导入文件 |
| 导出字段 | IP、状态、平均延迟、最小延迟、最大延迟、TTL、丢包率、时间戳 |

### 3.6 配置管理

| 功能 | 描述 |
|------|------|
| 持久化存储 | JSON 格式配置文件 |
| 配置文件路径 | Windows: `%APPDATA%\PingTool\config.json`<br>Linux: `~/.config/PingTool/config.json` |
| 记住设置 | 超时、包大小、间隔、并发数等 |
| 最近文件 | 记住最近导入的 10 个文件 |

---

## 4. UI 规格

### 4.1 窗口布局

```
┌─────────────────────────────────────────────────────────────┐
│  PingTool                                       [─][□][×]  │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  Target Input                                          │ │
│  │  [Input IP/Domain/CIDR]        [Add] [Clear]           │ │
│  │  [Batch input area              ]                     │ │
│  │  [Drag TXT/Excel file here     ]                     │ │
│  └─────────────────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  Control Parameters                                    │ │
│  │  [Start] [Stop]  Timeout:[2000ms] Packet:[64B]       │ │
│  │  Interval:[0ms] Concurrency:[100] Count:[4]           │ │
│  │                          [Import] [Export]            │ │
│  └─────────────────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  Progress: ████████████████████░░░░░░░ 99.2%          │ │
│  │  Online: 850 | Offline: 142 | Total: 1000             │ │
│  └─────────────────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  Results                              [Sort by ▼]     │ │
│  │  ☐ | IP/Domain     | Status | Avg | Min | Max | TTL │ │
│  │  ──┼────────────────┼────────┼─────┼─────┼─────┼──────│ │
│  │  ☑ | 192.168.1.1    |  [OK]  | 1.2 | 0.8 | 2.1 | 64  │ │
│  │  ☑ | 8.8.8.8        |  [OK]  |  15 |  12 |  22 | 112 │ │
│  │  ☐ | 192.168.1.256  | [FAIL] |  -  |  -  |  -  |  -  │ │
│  └─────────────────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  Log (collapsible)                       [▼ Collapse]  │ │
│  │  [12:30:15] INFO  Scan complete, 850/1000 online     │ │
│  └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 配色方案 (深色科技风)

| 元素 | 颜色 | 说明 |
|------|------|------|
| 背景色 | `#0d1117` | 主背景 (GitHub Dark) |
| 卡片背景 | `#161b22` | 面板背景 |
| 边框色 | `#30363d` | 分割线 |
| 主色调 | `#238636` | 按钮/成功 (绿色系) |
| 强调色 | `#58a6ff` | 链接/高亮 (蓝色) |
| 在线状态 | `#3fb950` | 成功/在线 |
| 离线状态 | `#f85149` | 失败/离线 |
| 警告状态 | `#d29922` | 警告 |
| 文字主色 | `#c9d1d9` | 主文字 |
| 文字次色 | `#8b949e` | 次要文字 |

### 4.3 字体

- 主字体: `-apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif`
- 等宽字体: `"SF Mono", "Consolas", "Monaco", monospace`

---

## 5. 数据模型

### 5.1 Rust 端

```rust
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PingResult {
    pub ip: String,
    pub raw_ip: Option<String>,
    pub status: Status,
    pub avg_rtt: f64,
    pub min_rtt: f64,
    pub max_rtt: f64,
    pub ttl: Option<u32>,
    pub packet_loss: f64,
    pub timestamp: String,
    pub error_msg: Option<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Status {
    Online,
    Timeout,
    Error,
    ResolveFailed,
    Pending,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Config {
    pub timeout: u32,
    pub packet_size: u32,
    pub interval: u32,
    pub concurrency: u32,
    pub ping_count: u32,
    pub recent_files: Vec<String>,
}
```

### 5.2 前端 (TypeScript)

```typescript
interface PingResult {
  ip: string;
  raw_ip?: string;
  status: 'Online' | 'Timeout' | 'Error' | 'ResolveFailed' | 'Pending';
  avg_rtt: number;
  min_rtt: number;
  max_rtt: number;
  ttl?: number;
  packet_loss: number;
  timestamp: string;
  error_msg?: string;
}

interface Config {
  timeout: number;
  packet_size: number;
  interval: number;
  concurrency: number;
  ping_count: number;
  recent_files: string[];
}
```

---

## 6. 项目结构

```
D:/workspace/PingTool/
├── src-tauri/
│   ├── src/
│   │   ├── main.rs           # Tauri 入口
│   │   ├── lib.rs           # 库入口
│   │   ├── commands.rs      # Tauri 命令
│   │   ├── ping.rs          # Ping 引擎
│   │   ├── config.rs        # 配置管理
│   │   └── excel.rs         # Excel 处理
│   ├── Cargo.toml
│   ├── tauri.conf.json
│   └── build.rs
├── src/
│   ├── main.ts              # 前端入口
│   ├── App.tsx              # React 组件
│   ├── components/
│   │   ├── InputPanel.tsx   # IP 输入面板
│   │   ├── ControlBar.tsx   # 控制栏
│   │   ├── ResultTable.tsx  # 结果表格
│   │   └── LogPanel.tsx     # 日志面板
│   ├── styles/
│   │   └── global.css       # 全局样式
│   └── types/
│       └── index.ts         # 类型定义
├── package.json
├── vite.config.ts
├── tsconfig.json
├── index.html
├── SPEC.md
└── README.md
```

### 构建产物

| 平台 | 输出文件 |
|------|----------|
| Windows | `PingTool.exe` (~10MB) |
| Linux x64 | `PingTool` (~12MB) |
| Linux ARM64 | `PingTool` (~12MB) |

---

## 7. 关键算法

### 7.1 CIDR 扩展

```rust
fn expand_cidr(cidr: &str) -> Result<Vec<String>, Error> {
    let ip = cidr.parse::<Ipv4Addr>()?;
    let mask = cidr.split('/').nth(1).unwrap().parse::<u8>()?;
    let num_ips = 1u32 << (32 - mask);

    (0..num_ips)
        .map(|i| {
            let ip_u32 = u32::from(ip);
            Ipv4Addr::from(ip_u32 + i).to_string()
        })
        .collect()
}
```

### 7.2 异步并发控制

```rust
use tokio::sync::Semaphore;

async fn ping_targets(targets: Vec<String>, config: &Config) {
    let sem = Arc::new(Semaphore::new(config.concurrency as usize));

    let handles: Vec<_> = targets.into_iter().map(|target| {
        let sem = Arc::clone(&sem);
        async move {
            let _permit = sem.acquire().await;
            ping_single(&target, config).await
        }
    }).collect();

    futures::future::join_all(handles).await;
}
```

### 7.3 Ping 输出解析

**Windows (`ping -n 1 -w 2000 ip`):**
```
Reply from 8.8.8.8: bytes=32 time=15ms TTL=112
Request timed out.
```

**Linux (`ping -c 1 -W 2 ip`):**
```
64 bytes from 8.8.8.8: icmp_seq=1 ttl=112 time=15.2 ms
```

---

## 8. 跨平台兼容

| 平台 | 命令 | 超时参数 |
|------|------|----------|
| Windows | `ping -n {count} -w {timeout_ms}` | `-w` 毫秒 |
| Linux | `ping -c {count} -W {timeout_sec}` | `-W` 秒 |

---

## 9. Tauri 命令接口

| 命令 | 描述 | 参数 |
|------|------|------|
| `start_ping` | 开始 ping | `targets: Vec<String>, config: Config` |
| `stop_ping` | 停止 ping | - |
| `parse_input` | 解析 IP/CIDR | `input: String` |
| `import_file` | 导入文件 | `path: String` |
| `export_excel` | 导出 Excel | `path: String, results: Vec<PingResult>` |
| `get_config` | 获取配置 | - |
| `save_config` | 保存配置 | `config: Config` |

---

## 10. 验收标准

### 10.1 功能验收

- [ ] 支持单 IP 输入并正确 Ping
- [ ] 支持 CIDR 范围展开 (e.g., /24)
- [ ] 支持域名解析并 Ping
- [ ] 支持 1000+ IP 并发探测，响应不卡顿
- [ ] TXT 文件拖拽导入成功
- [ ] Excel 文件导入，正确识别 IP 列
- [ ] 结果表格所有列可排序
- [ ] 导出结果到 Excel 文件
- [ ] 回写结果到源 Excel 文件
- [ ] 配置项保存并在下一次启动时恢复

### 10.2 UI 验收

- [ ] 深色科技风格，符合配色规格
- [ ] 窗口可调整大小，最小 800x600
- [ ] 实时显示进度和统计
- [ ] 状态图标正确显示
- [ ] 表格滚动流畅 (1000+ 行)

### 10.3 跨平台验收

- [ ] Windows 10/11 运行 EXE 正常
- [ ] Linux ARM (Raspberry Pi) 运行正常

---

## 11. 未来扩展 (v2.0)

- [ ] TCPing / HTTPPing 支持
- [ ] 探测历史记录数据库
- [ ] 探测结果趋势图
- [ ] 告警规则
- [ ] 多语言支持

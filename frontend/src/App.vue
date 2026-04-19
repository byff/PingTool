<script setup lang="ts">
import { ref, computed, onMounted } from 'vue'
import { Modal, Slider, message } from 'ant-design-vue'
import {
  PlayCircleOutlined,
  StopOutlined,
  ReloadOutlined,
  ImportOutlined,
  ExportOutlined,
  SettingOutlined,
  InfoCircleOutlined,
  UploadOutlined,
  InsertRowAboveOutlined
} from '@ant-design/icons-vue'

interface PingResult {
  num: number
  hostname: string
  ip: string
  success: number
  fail: number
  fail_pct: string
  total: number
  rtt: string
  rtt_max: string
  rtt_min: string
  rtt_avg: string
  status: number
}

interface Config {
  timeout: number
  packet_size: number
  interval: number
  max_concurrent: number
  cidr_enabled: boolean
  remember_ips: boolean
  ip_list: string
}

const inputText = ref('')
const isRunning = ref(false)
const results = ref<PingResult[]>([])
const selectedCount = ref(0)
const statusMessage = ref('就绪')

const settingsVisible = ref(false)
const aboutVisible = ref(false)
const importModalVisible = ref(false)

const config = ref<Config>({
  timeout: 1000,
  packet_size: 64,
  interval: 1000,
  max_concurrent: 100,
  cidr_enabled: true,
  remember_ips: true,
  ip_list: ''
})

const sourceFilePath = ref('')

const stats = computed(() => {
  const total = results.value.length
  const success = results.value.filter(r => r.status === 0).length
  const fail = results.value.filter(r => r.status === 2).length
  const avgRtt = results.value
    .filter(r => r.rtt !== '-')
    .reduce((sum, r) => {
      const match = r.rtt.match(/(\d+)/)
      return sum + (match ? parseInt(match[1]) : 0)
    }, 0) / (total || 1)
  return { total, success, fail, avgRtt: Math.round(avgRtt) }
})

onMounted(async () => {
  try {
    const loaded = await window.go.pingtool.App.LoadConfig()
    config.value = loaded
    if (loaded.ip_list && config.value.remember_ips) {
      inputText.value = loaded.ip_list
    }
  } catch (e) {
    console.error('Failed to load config:', e)
  }
})

async function startPing() {
  if (isRunning.value) return

  const targets = inputText.value.trim().split('\n').filter(t => t.trim())
  if (targets.length === 0) {
    message.warning('请输入要 Ping 的目标')
    return
  }

  if (targets.length > 1000) {
    Modal.confirm({
      title: '警告',
      content: `您输入了 ${targets.length} 个目标，这可能会导致高内存占用和性能下降。确定要继续吗？`,
      okText: '继续',
      cancelText: '取消',
      onOk: () => doStart(targets)
    })
  } else {
    doStart(targets)
  }
}

async function doStart(targets: string[]) {
  isRunning.value = true
  statusMessage.value = '正在 Ping...'
  results.value = []

  try {
    await window.go.pingtool.App.StartPing(
      targets,
      config.value.timeout,
      config.value.packet_size,
      config.value.interval,
      config.value.max_concurrent
    )
  } catch (e) {
    message.error('启动 Ping 失败: ' + e)
    isRunning.value = false
    statusMessage.value = '启动失败'
    return
  }

  const pollInterval = setInterval(async () => {
    if (!isRunning.value) {
      clearInterval(pollInterval)
      return
    }
    try {
      const newResults = await window.go.pingtool.App.GetResults()
      if (newResults && newResults.length > 0) {
        results.value = newResults.map((r: any) => ({
          num: r.Num || 0,
          hostname: r.Hostname || '',
          ip: r.IP || '',
          success: r.Success || 0,
          fail: r.Fail || 0,
          fail_pct: r.FailPct || '-',
          total: r.Total || 0,
          rtt: r.RTT || '-',
          rtt_max: r.RTTMax || '-',
          rtt_min: r.RTTMin || '-',
          rtt_avg: r.RTTAvg || '-',
          status: r.Status || -1
        }))
        selectedCount.value = results.value.length
        statusMessage.value = `已 Ping ${results.value.length} 个目标`
      }
    } catch (e) {
      console.error('Failed to get results:', e)
    }
  }, 500)
}

async function stopPing() {
  if (!isRunning.value) return
  try {
    await window.go.pingtool.App.StopPing()
  } catch (e) {
    console.error('Failed to stop ping:', e)
  }
  isRunning.value = false
  statusMessage.value = '已停止'
}

async function refreshResults() {
  try {
    const newResults = await window.go.pingtool.App.GetResults()
    if (newResults && newResults.length > 0) {
      results.value = newResults.map((r: any) => ({
        num: r.Num || 0,
        hostname: r.Hostname || '',
        ip: r.IP || '',
        success: r.Success || 0,
        fail: r.Fail || 0,
        fail_pct: r.FailPct || '-',
        total: r.Total || 0,
        rtt: r.RTT || '-',
        rtt_max: r.RTTMax || '-',
        rtt_min: r.RTTMin || '-',
        rtt_avg: r.RTTAvg || '-',
        status: r.Status || -1
      }))
      selectedCount.value = results.value.length
    }
  } catch (e) {
    console.error('Failed to refresh results:', e)
  }
}

function clearResults() {
  results.value = []
  selectedCount.value = 0
  statusMessage.value = '已清空'
}

async function importFile() {
  importModalVisible.value = true
}

async function handleFileImport(file: File) {
  const path = (file as any).path || file.name
  try {
    let ips: string[] = []
    if (path.endsWith('.txt')) {
      ips = await window.go.pingtool.App.ImportTxt(path)
    } else if (path.endsWith('.xlsx') || path.endsWith('.xls')) {
      const result = await window.go.pingtool.App.ImportExcel(path)
      ips = result.ips
      sourceFilePath.value = path
    }
    if (ips.length > 0) {
      inputText.value = ips.join('\n')
      message.success(`成功导入 ${ips.length} 个目标`)
    }
  } catch (e) {
    message.error('导入失败: ' + e)
  }
  importModalVisible.value = false
}

function exportResults() {
  if (results.value.length === 0) {
    message.warning('没有可导出的结果')
    return
  }
  const path = `ping_results_${Date.now()}.xlsx`
  try {
    const exportData = results.value.map(r => ({
      Num: r.num,
      Hostname: r.hostname,
      IP: r.ip,
      Success: r.success,
      Fail: r.fail,
      FailPct: r.fail_pct,
      Total: r.total,
      RTT: r.rtt,
      RTTMax: r.rtt_max,
      RTTMin: r.rtt_min,
      RTTAvg: r.rtt_avg,
      Status: r.status
    }))
    window.go.pingtool.App.ExportExcel(exportData, path)
    message.success(`已导出到 ${path}`)
  } catch (e) {
    message.error('导出失败: ' + e)
  }
}

function insertToSource() {
  if (results.value.length === 0) {
    message.warning('没有可插入的结果')
    return
  }
  if (!sourceFilePath.value) {
    message.warning('请先导入 Excel 文件')
    return
  }
  try {
    const exportData = results.value.map(r => ({
      Num: r.num,
      Hostname: r.hostname,
      IP: r.ip,
      Success: r.success,
      Fail: r.fail,
      FailPct: r.fail_pct,
      Total: r.total,
      RTT: r.rtt,
      RTTMax: r.rtt_max,
      RTTMin: r.rtt_min,
      RTTAvg: r.rtt_avg,
      Status: r.status
    }))
    window.go.pingtool.App.InsertToSource(exportData, sourceFilePath.value, 0)
    message.success('已插入到源文件')
  } catch (e) {
    message.error('插入失败: ' + e)
  }
}

function openSettings() {
  settingsVisible.value = true
}

function openAbout() {
  aboutVisible.value = true
}

async function saveSettings() {
  try {
    if (config.value.remember_ips) {
      config.value.ip_list = inputText.value
    }
    await window.go.pingtool.App.SaveConfig(config.value)
    message.success('设置已保存')
  } catch (e) {
    message.error('保存失败: ' + e)
  }
  settingsVisible.value = false
}

function resetSettings() {
  config.value = {
    timeout: 1000,
    packet_size: 64,
    interval: 1000,
    max_concurrent: 100,
    cidr_enabled: true,
    remember_ips: true,
    ip_list: ''
  }
}

function handleDrop(e: DragEvent) {
  e.preventDefault()
  const files = e.dataTransfer?.files
  if (files && files.length > 0) {
    handleFileImport(files[0])
  }
}

function handleDragOver(e: DragEvent) {
  e.preventDefault()
}

function getStatusClass(status: number): string {
  switch (status) {
    case 0: return 'status-alive'
    case 1: return 'status-partial'
    case 2: return 'status-failed'
    default: return 'status-unknown'
  }
}
</script>

<template>
  <div class="app-container">
    <div class="toolbar">
      <div class="toolbar-left">
        <span class="logo">PingTool</span>
        <a-button
          type="primary"
          :disabled="isRunning"
          @click="startPing"
        >
          <PlayCircleOutlined /> 开始
        </a-button>
        <a-button
          danger
          :disabled="!isRunning"
          @click="stopPing"
        >
          <StopOutlined /> 停止
        </a-button>
        <a-button @click="refreshResults">
          <ReloadOutlined /> 刷新
        </a-button>
        <a-divider type="vertical" />
        <a-button @click="importFile">
          <ImportOutlined /> 导入
        </a-button>
        <a-button @click="exportResults">
          <ExportOutlined /> 导出结果
        </a-button>
        <a-button @click="insertToSource">
          <InsertRowAboveOutlined /> 插入源表
        </a-button>
      </div>
      <div class="toolbar-right">
        <a-button @click="openSettings">
          <SettingOutlined /> 设置
        </a-button>
        <a-button @click="openAbout">
          <InfoCircleOutlined /> 关于
        </a-button>
      </div>
    </div>

    <div class="main-content">
      <div class="input-section">
        <label>IP 地址 (支持 IP/CIDR/域名，换行分隔)</label>
        <textarea
          v-model="inputText"
          placeholder="192.168.1.1&#10;10.0.0.0/24&#10;google.com"
          @drop="handleDrop"
          @dragover="handleDragOver"
        ></textarea>
      </div>

      <div class="table-section">
        <div class="table-header">
          <div class="table-cell col-num">#</div>
          <div class="table-cell col-hostname">主机名</div>
          <div class="table-cell col-ip">IP</div>
          <div class="table-cell col-success">成功</div>
          <div class="table-cell col-fail">失败</div>
          <div class="table-cell col-fail-pct">失败率</div>
          <div class="table-cell col-total">总计</div>
          <div class="table-cell col-rtt">延迟</div>
          <div class="table-cell col-max">最大</div>
          <div class="table-cell col-min">最小</div>
          <div class="table-cell col-avg">平均</div>
        </div>
        <div class="table-body">
          <div
            v-for="row in results"
            :key="row.num"
            class="table-row"
          >
            <div class="table-cell col-num">{{ row.num }}</div>
            <div class="table-cell col-hostname">{{ row.hostname }}</div>
            <div class="table-cell col-ip" :class="getStatusClass(row.status)">{{ row.ip }}</div>
            <div class="table-cell col-success">{{ row.success }}</div>
            <div class="table-cell col-fail">{{ row.fail }}</div>
            <div class="table-cell col-fail-pct">{{ row.fail_pct }}</div>
            <div class="table-cell col-total">{{ row.total }}</div>
            <div class="table-cell col-rtt">{{ row.rtt }}</div>
            <div class="table-cell col-max">{{ row.rtt_max }}</div>
            <div class="table-cell col-min">{{ row.rtt_min }}</div>
            <div class="table-cell col-avg">{{ row.rtt_avg }}</div>
          </div>
          <div v-if="results.length === 0" style="padding: 40px; text-align: center; color: var(--text-dim);">
            暂无数据
          </div>
        </div>
      </div>
    </div>

    <div class="status-bar">
      <span>{{ statusMessage }}</span>
      <div class="status-item">
        <span>总计:</span>
        <span>{{ stats.total }}</span>
      </div>
      <div class="status-item">
        <span>成功:</span>
        <span style="color: var(--success)">{{ stats.success }}</span>
      </div>
      <div class="status-item">
        <span>失败:</span>
        <span style="color: var(--fail)">{{ stats.fail }}</span>
      </div>
      <div class="status-item">
        <span>平均延迟:</span>
        <span style="color: var(--accent)">{{ stats.avgRtt }}ms</span>
      </div>
    </div>

    <a-modal
      v-model:open="settingsVisible"
      title="设置"
      class="settings-modal"
      :footer="null"
      width="500px"
    >
      <div class="settings-section">
        <div class="settings-section-title">Ping 参数</div>
        <div class="settings-row">
          <label>超时时间</label>
          <div class="slider-control">
            <a-slider
              v-model:value="config.timeout"
              :min="100"
              :max="10000"
              :step="100"
              style="width: 200px"
            />
            <span>{{ config.timeout }}ms</span>
          </div>
        </div>
        <div class="settings-row">
          <label>数据包大小</label>
          <div class="slider-control">
            <a-slider
              v-model:value="config.packet_size"
              :min="32"
              :max="65500"
              :step="32"
              style="width: 200px"
            />
            <span>{{ config.packet_size }} B</span>
          </div>
        </div>
        <div class="settings-row">
          <label>Ping 间隔</label>
          <div class="slider-control">
            <a-slider
              v-model:value="config.interval"
              :min="100"
              :max="60000"
              :step="100"
              style="width: 200px"
            />
            <span>{{ config.interval }}ms</span>
          </div>
        </div>
        <div class="settings-row">
          <label>最大并发数</label>
          <div class="slider-control">
            <a-slider
              v-model:value="config.max_concurrent"
              :min="1"
              :max="500"
              :step="1"
              style="width: 200px"
            />
            <span>{{ config.max_concurrent }}</span>
          </div>
        </div>
      </div>

      <div class="settings-section">
        <div class="settings-section-title">选项</div>
        <div class="settings-row">
          <label>启用 CIDR 表示法</label>
          <a-switch v-model:checked="config.cidr_enabled" />
        </div>
        <div class="settings-row">
          <label>记住输入的 IP 列表</label>
          <a-switch v-model:checked="config.remember_ips" />
        </div>
      </div>

      <div style="display: flex; gap: 8px; justify-content: flex-end;">
        <a-button @click="resetSettings">重置</a-button>
        <a-button type="primary" @click="saveSettings">保存</a-button>
      </div>
    </a-modal>

    <a-modal
      v-model:open="aboutVisible"
      title="关于"
      class="settings-modal"
      :footer="null"
      width="360px"
    >
      <div style="text-align: center; padding: 20px 0;">
        <h2 style="color: var(--accent); margin-bottom: 8px;">PingTool</h2>
        <p style="color: var(--text-dim); margin-bottom: 16px;">版本 v1.0.0</p>
        <a-divider />
        <p style="color: #fff; font-size: 13px; line-height: 1.6;">
          高性能多目标 Ping 工具<br/>
          支持同时 Ping 1000+ 个目标
        </p>
        <a-divider />
        <p style="color: var(--text-dim); font-size: 12px;">
          版权所有 2026
        </p>
      </div>
      <div style="text-align: center;">
        <a-button type="primary" @click="aboutVisible = false">关闭</a-button>
      </div>
    </a-modal>

    <a-modal
      v-model:open="importModalVisible"
      title="导入文件"
      class="settings-modal"
      :footer="null"
      width="400px"
    >
      <div
        class="dropzone"
        @drop="handleDrop"
        @dragover="handleDragOver"
      >
        <UploadOutlined style="font-size: 48px; color: var(--accent); margin-bottom: 16px;" />
        <p style="color: #fff; margin-bottom: 8px;">拖拽文件到此处</p>
        <p style="color: var(--text-dim); font-size: 12px;">支持 .txt 和 .xlsx 文件</p>
      </div>
      <p style="color: var(--text-dim); font-size: 12px; text-align: center; margin-top: 16px;">
        Excel 文件会自动识别 IP 列
      </p>
    </a-modal>
  </div>
</template>

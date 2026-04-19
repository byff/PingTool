import { createApp } from 'vue'
import Antd from 'ant-design-vue'
import App from './App.vue'
import 'ant-design-vue/dist/reset.css'
import './styles/theme.css'

const app = createApp(App)
app.use(Antd)
app.mount('#app')

declare global {
  interface Window {
    go: {
      pingtool: {
        App: {
          StartPing: (targets: string[], timeout: number, packetSize: number, interval: number, concurrent: number) => Promise<void>
          StopPing: () => Promise<void>
          GetResults: () => Promise<any[]>
          ParseTargets: (input: string) => Promise<string[]>
          ImportTxt: (path: string) => Promise<string[]>
          ImportExcel: (path: string) => Promise<{ ips: string[], colIndex: number }>
          ExportExcel: (results: any[], path: string) => Promise<void>
          InsertToSource: (results: any[], sourcePath: string, colIndex: number) => Promise<void>
          LoadConfig: () => Promise<any>
          SaveConfig: (config: any) => Promise<void>
        }
      }
    }
  }
}

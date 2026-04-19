export interface PingResult {
  ip: string;
  rawIP?: string;
  status: 'Online' | 'Timeout' | 'Error' | 'ResolveFailed' | 'Pending';
  avgRtt: number;
  minRtt: number;
  maxRtt: number;
  ttl?: number;
  packetLoss: number;
  timestamp: string;
  errorMsg?: string;
}

export interface ProgressInfo {
  total: number;
  completed: number;
  online: number;
  offline: number;
  percent: number;
}

export interface ProgressEvent {
  result: PingResult;
  progress: ProgressInfo;
}

export interface Config {
  timeout: number;
  packet_size: number;
  interval: number;
  concurrency: number;
  ping_count: number;
  recent_files: string[];
}

export interface StartPingRequest {
  targets: string[];
  timeout: number;
  packet_size: number;
  interval: number;
  concurrency: number;
  ping_count: number;
}

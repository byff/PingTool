import { useState, useEffect, useCallback, useRef } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { listen } from '@tauri-apps/event';
import { open, save } from '@tauri-apps/plugin-dialog';
import type { PingResult, ProgressEvent, Config } from './types';

function App() {
  const [targets, setTargets] = useState<string[]>([]);
  const [inputSingle, setInputSingle] = useState('');
  const [inputMulti, setInputMulti] = useState('');
  const [results, setResults] = useState<PingResult[]>([]);
  const [isRunning, setIsRunning] = useState(false);
  const [progress, setProgress] = useState({ total: 0, completed: 0, online: 0, offline: 0, percent: 0 });
  const [logs, setLogs] = useState<Array<{ type: string; msg: string }>>([]);
  const [sortColumn, setSortColumn] = useState<string>('ip');
  const [sortOrder, setSortOrder] = useState<'asc' | 'desc'>('asc');
  const [config, setConfig] = useState<Config>({
    timeout: 2000,
    packet_size: 64,
    interval: 0,
    concurrency: 100,
    ping_count: 4,
    recent_files: [],
  });

  const [timeout, setTimeout] = useState(2000);
  const [packetSize, setPacketSize] = useState(64);
  const [interval, setInterval] = useState(0);
  const [concurrency, setConcurrency] = useState(100);
  const [pingCount, setPingCount] = useState(4);

  useEffect(() => {
    loadConfig();
    const unlisten = listen<ProgressEvent>('ping-progress', (event) => {
      const { result, progress } = event.payload;
      setResults((prev) => [...prev, result]);
      setProgress(progress);
    });
    return () => {
      unlisten.then((fn) => fn());
    };
  }, []);

  const loadConfig = async () => {
    try {
      const cfg = await invoke<Config>('get_config');
      setConfig(cfg);
      setTimeout(cfg.timeout);
      setPacketSize(cfg.packet_size);
      setInterval(cfg.interval);
      setConcurrency(cfg.concurrency);
      setPingCount(cfg.ping_count);
    } catch (e) {
      console.error('Failed to load config:', e);
    }
  };

  const addLog = (msg: string, type: string = 'info') => {
    const time = new Date().toLocaleTimeString();
    setLogs((prev) => [...prev, { type, msg: `[${time}] ${msg}` }]);
  };

  const handleAddTargets = async () => {
    const input = inputSingle.trim();
    if (input) {
      try {
        const ips = await invoke<string[]>('parse_input', { input });
        setTargets((prev) => [...new Set([...prev, ...ips])]);
        addLog(`Added ${ips.length} targets from input`);
      } catch (e) {
        addLog(`Failed to parse input: ${e}`, 'error');
      }
    }

    const multiInput = inputMulti.trim();
    if (multiInput) {
      try {
        const ips = await invoke<string[]>('parse_input', { input: multiInput });
        setTargets((prev) => [...new Set([...prev, ...ips])]);
        addLog(`Added ${ips.length} targets from batch input`);
      } catch (e) {
        addLog(`Failed to parse batch input: ${e}`, 'error');
      }
    }

    setInputSingle('');
    setInputMulti('');
  };

  const handleClearTargets = () => {
    setTargets([]);
    setResults([]);
    setProgress({ total: 0, completed: 0, online: 0, offline: 0, percent: 0 });
    addLog('Cleared all targets');
  };

  const handleStartPing = async () => {
    if (targets.length === 0) {
      addLog('No targets to ping', 'error');
      return;
    }

    setIsRunning(true);
    setResults([]);
    addLog(`Starting ping for ${targets.length} targets...`);

    try {
      const pingResults = await invoke<PingResult[]>('start_ping', {
        request: {
          targets,
          timeout,
          packet_size: packetSize,
          interval,
          concurrency,
          ping_count: pingCount,
        },
      });
      setResults(pingResults);
      addLog(`Ping complete, ${pingResults.length} results`, 'success');
    } catch (e) {
      addLog(`Ping failed: ${e}`, 'error');
    }

    setIsRunning(false);
  };

  const handleStopPing = async () => {
    try {
      await invoke('stop_ping');
      addLog('Stopping ping...');
    } catch (e) {
      addLog(`Failed to stop: ${e}`, 'error');
    }
  };

  const handleImport = async () => {
    try {
      const selected = await open({
        multiple: false,
        filters: [{ name: 'IP Files', extensions: ['txt', 'xlsx', 'xls'] }],
      });
      if (selected) {
        const ips = await invoke<string[]>('import_file', { path: selected });
        setTargets((prev) => [...new Set([...prev, ...ips])]);
        addLog(`Imported ${ips.length} IPs from file`);
      }
    } catch (e) {
      addLog(`Import failed: ${e}`, 'error');
    }
  };

  const handleExport = async () => {
    if (results.length === 0) {
      addLog('No results to export', 'error');
      return;
    }

    try {
      const path = await save({
        filters: [{ name: 'Excel', extensions: ['xlsx'] }],
        defaultPath: 'ping_results.xlsx',
      });
      if (path) {
        await invoke('export_excel', { path, results });
        addLog(`Exported ${results.length} results to ${path}`, 'success');
      }
    } catch (e) {
      addLog(`Export failed: ${e}`, 'error');
    }
  };

  const handleSort = (column: string) => {
    if (sortColumn === column) {
      setSortOrder(sortOrder === 'asc' ? 'desc' : 'asc');
    } else {
      setSortColumn(column);
      setSortOrder('asc');
    }
  };

  const sortedResults = [...results].sort((a, b) => {
    let va: any = (a as any)[sortColumn];
    let vb: any = (b as any)[sortColumn];
    if (typeof va === 'string') {
      return sortOrder === 'asc' ? va.localeCompare(vb) : vb.localeCompare(va);
    }
    return sortOrder === 'asc' ? va - vb : vb - va;
  });

  const formatStatus = (status: string) => {
    switch (status) {
      case 'Online': return <span className="status-badge status-online">[OK]</span>;
      case 'Timeout': return <span className="status-badge status-offline">[TIMEOUT]</span>;
      case 'Error': return <span className="status-badge status-error">[ERROR]</span>;
      case 'ResolveFailed': return <span className="status-badge status-error">[DNS_FAIL]</span>;
      default: return <span className="status-badge">[{status}]</span>;
    }
  };

  return (
    <div className="app">
      <header className="header">
        <div className="logo">
          <span className="logo-icon">*</span>
          <span className="logo-text">PingTool</span>
        </div>
      </header>

      <main className="main">
        <section className="section">
          <div className="section-title">TARGET INPUT</div>
          <div className="input-row">
            <input
              type="text"
              className="input"
              placeholder="Input IP/Domain/CIDR (e.g. 192.168.1.0/24)"
              value={inputSingle}
              onChange={(e) => setInputSingle(e.target.value)}
              onKeyPress={(e) => e.key === 'Enter' && handleAddTargets()}
            />
            <button className="btn btn-primary" onClick={handleAddTargets}>Add</button>
            <button className="btn btn-secondary" onClick={handleClearTargets}>Clear</button>
          </div>
          <textarea
            className="textarea"
            placeholder="Batch input, one per line (IP/Domain/CIDR)"
            value={inputMulti}
            onChange={(e) => setInputMulti(e.target.value)}
          />
          <div className="drop-zone">Drag TXT/Excel file here to import</div>
        </section>

        <section className="section">
          <div className="section-title">CONTROL PARAMETERS</div>
          <div className="control-row">
            <div className="control-group">
              <button
                className="btn btn-success btn-large"
                onClick={handleStartPing}
                disabled={isRunning}
              >
                Start
              </button>
              <button
                className="btn btn-danger btn-large"
                onClick={handleStopPing}
                disabled={!isRunning}
              >
                Stop
              </button>
            </div>
            <div className="param-group">
              <div className="param">
                <label>Timeout(ms)</label>
                <input
                  type="number"
                  value={timeout}
                  onChange={(e) => setTimeout(Number(e.target.value))}
                  min={100}
                  max={10000}
                />
              </div>
              <div className="param">
                <label>Packet(B)</label>
                <input
                  type="number"
                  value={packetSize}
                  onChange={(e) => setPacketSize(Number(e.target.value))}
                  min={32}
                  max={65535}
                />
              </div>
              <div className="param">
                <label>Interval(ms)</label>
                <input
                  type="number"
                  value={interval}
                  onChange={(e) => setInterval(Number(e.target.value))}
                  min={0}
                  max={10000}
                />
              </div>
              <div className="param">
                <label>Concurrency</label>
                <input
                  type="number"
                  value={concurrency}
                  onChange={(e) => setConcurrency(Number(e.target.value))}
                  min={1}
                  max={1000}
                />
              </div>
              <div className="param">
                <label>Count</label>
                <input
                  type="number"
                  value={pingCount}
                  onChange={(e) => setPingCount(Number(e.target.value))}
                  min={1}
                  max={100}
                />
              </div>
            </div>
            <div className="control-group">
              <button className="btn btn-secondary" onClick={handleImport}>Import</button>
              <button className="btn btn-secondary" onClick={handleExport}>Export</button>
            </div>
          </div>
        </section>

        <section className="section">
          <div className="progress-bar">
            <div className="progress-fill" style={{ width: `${progress.percent}%` }} />
          </div>
          <div className="stats-row">
            <span>Status: {progress.completed}/{progress.total} ({progress.percent.toFixed(1)}%)</span>
            <span className="stat-online">Online: <strong>{progress.online}</strong></span>
            <span className="stat-offline">Offline: <strong>{progress.offline}</strong></span>
            <span>Total: <strong>{targets.length}</strong></span>
          </div>
        </section>

        <section className="section result-section">
          <div className="result-header">
            <div className="section-title">RESULTS</div>
            <div className="result-actions">
              <select
                className="select"
                value={sortColumn}
                onChange={(e) => handleSort(e.target.value)}
              >
                <option value="ip">Sort by IP</option>
                <option value="status">Sort by Status</option>
                <option value="avgRtt">Sort by Avg RTT</option>
                <option value="minRtt">Sort by Min RTT</option>
                <option value="maxRtt">Sort by Max RTT</option>
                <option value="ttl">Sort by TTL</option>
                <option value="packetLoss">Sort by Loss</option>
              </select>
            </div>
          </div>
          <div className="table-container">
            <table className="result-table">
              <thead>
                <tr>
                  <th className="col-ip" onClick={() => handleSort('ip')}>IP/Domain</th>
                  <th className="col-status" onClick={() => handleSort('status')}>Status</th>
                  <th className="col-rtt" onClick={() => handleSort('avgRtt')}>Avg(ms)</th>
                  <th className="col-rtt" onClick={() => handleSort('minRtt')}>Min(ms)</th>
                  <th className="col-rtt" onClick={() => handleSort('maxRtt')}>Max(ms)</th>
                  <th className="col-ttl" onClick={() => handleSort('ttl')}>TTL</th>
                  <th className="col-loss" onClick={() => handleSort('packetLoss')}>Loss(%)</th>
                  <th className="col-time">Time</th>
                </tr>
              </thead>
              <tbody>
                {sortedResults.map((r, i) => (
                  <tr key={i}>
                    <td className="col-ip">{r.ip}</td>
                    <td className="col-status">{formatStatus(r.status)}</td>
                    <td className="col-rtt">{r.avgRtt > 0 ? r.avgRtt.toFixed(1) : '-'}</td>
                    <td className="col-rtt">{r.minRtt > 0 && r.minRtt < 999999 ? r.minRtt.toFixed(1) : '-'}</td>
                    <td className="col-rtt">{r.maxRtt > 0 ? r.maxRtt.toFixed(1) : '-'}</td>
                    <td className="col-ttl">{r.ttl || '-'}</td>
                    <td className="col-loss">{r.packetLoss >= 0 ? r.packetLoss.toFixed(1) : '-'}</td>
                    <td className="col-time">{r.timestamp}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </section>

        <section className="section log-section">
          <div className="log-header">
            <div className="section-title">LOG</div>
          </div>
          <div className="log-content">
            {logs.map((log, i) => (
              <div key={i} className={`log-entry ${log.type}`}>{log.msg}</div>
            ))}
          </div>
        </section>
      </main>
    </div>
  );
}

export default App;

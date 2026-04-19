use crate::commands::ProgressInfo;
use anyhow::{anyhow, Result};
use regex::Regex;
use std::collections::HashSet;
use std::net::{Ipv4Addr, ToSocketAddrs};
use std::process::Command;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::time::Instant;
use tokio::sync::Semaphore;

#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct PingResult {
    pub ip: String,
    #[serde(rename = "rawIP")]
    pub raw_ip: Option<String>,
    pub status: String,
    #[serde(rename = "avgRtt")]
    pub avg_rtt: f64,
    #[serde(rename = "minRtt")]
    pub min_rtt: f64,
    #[serde(rename = "maxRtt")]
    pub max_rtt: f64,
    pub ttl: Option<u32>,
    #[serde(rename = "packetLoss")]
    pub packet_loss: f64,
    pub timestamp: String,
    #[serde(rename = "errorMsg")]
    pub error_msg: Option<String>,
}

pub struct PingConfig {
    pub timeout: u32,
    pub packet_size: u32,
    pub interval: u32,
    pub concurrency: u32,
    pub ping_count: u32,
}

pub fn parse_input(input: &str) -> Result<Vec<String>> {
    let mut ips = Vec::new();
    let mut seen = HashSet::new();

    for line in input.lines() {
        let line = line.trim();
        if line.is_empty() {
            continue;
        }

        let expanded = if line.contains('/') {
            expand_cidr(line)?
        } else {
            vec![line.to_string()]
        };

        for ip in expanded {
            if !seen.contains(&ip) {
                seen.insert(ip.clone());
                ips.push(ip);
            }
        }
    }

    if ips.len() > 10000 {
        return Err(anyhow!("Too many IPs: {} (max 10000)", ips.len()));
    }

    Ok(ips)
}

fn expand_cidr(cidr: &str) -> Result<Vec<String>> {
    let parts: Vec<&str> = cidr.split('/').collect();
    if parts.len() != 2 {
        return Err(anyhow!("Invalid CIDR format: {}", cidr));
    }

    let ip: Ipv4Addr = parts[0].parse().map_err(|_| anyhow!("Invalid IP: {}", parts[0]))?;
    let mask: u8 = parts[1].parse().map_err(|_| anyhow!("Invalid mask: {}", parts[1]))?;

    if mask > 32 {
        return Err(anyhow!("Invalid mask: {}", mask));
    }

    let num_ips = 1u32 << (32 - mask);
    if num_ips > 10000 {
        return Err(anyhow!("CIDR range too large: {} IPs", num_ips));
    }

    let ip_u32 = u32::from(ip);
    let mut result = Vec::with_capacity(num_ips as usize);

    for i in 0..num_ips {
        let addr = Ipv4Addr::from(ip_u32 + i);
        result.push(addr.to_string());
    }

    Ok(result)
}

fn is_valid_ip(s: &str) -> bool {
    s.parse::<Ipv4Addr>().is_ok()
}

fn resolve_domain(domain: &str) -> Option<String> {
    match format!("{}:80", domain).to_socket_addrs() {
        Ok(mut addrs) => {
            if let Some(addr) = addrs.next() {
                return Some(addr.ip().to_string());
            }
        }
        Err(_) => {}
    }

    #[cfg(windows)]
    {
        let output = Command::new("nslookup")
            .arg(domain)
            .output()
            .ok()?;

        let stdout = String::from_utf8_lossy(&output.stdout);
        let ip_regex = Regex::new(r"(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})").ok()?;

        for cap in ip_regex.captures_iter(&stdout) {
            if let Some(ip) = cap.get(1) {
                let ip_str = ip.as_str();
                if is_valid_ip(ip_str) && !ip_str.starts_with("127.") {
                    return Some(ip_str.to_string());
                }
            }
        }
    }

    None
}

fn parse_ping_output(output: &str, is_windows: bool) -> Option<(u32, f64)> {
    let time_regex = Regex::new(r"time[=<](\d+(?:\.\d+)?)\s*ms").ok()?;
    let ttl_regex = Regex::new(r"TTL=(\d+)").ok()?;

    let time_cap = time_regex.captures(output)?;
    let time_str = time_cap.get(1)?.as_str();
    let time_ms: f64 = time_str.parse().ok()?;

    let ttl = ttl_regex
        .captures(output)
        .and_then(|c| c.get(1))
        .and_then(|m| m.as_str().parse().ok());

    Some((ttl.unwrap_or(64), time_ms))
}

fn ping_single(target: &str, config: &PingConfig, is_windows: bool) -> PingResult {
    let timestamp = chrono_lite_timestamp();

    let mut raw_ip: Option<String> = None;
    let is_domain = !is_valid_ip(target);

    if is_domain {
        if let Some(ip) = resolve_domain(target) {
            raw_ip = Some(ip.clone());
        } else {
            return PingResult {
                ip: target.to_string(),
                raw_ip: None,
                status: "ResolveFailed".to_string(),
                avg_rtt: 0.0,
                min_rtt: 0.0,
                max_rtt: 0.0,
                ttl: None,
                packet_loss: 100.0,
                timestamp,
                error_msg: Some("DNS resolution failed".to_string()),
            };
        }
    }

    let ping_target = raw_ip.as_ref().map(|s| s.as_str()).unwrap_or(target);

    let mut rtts: Vec<f64> = Vec::new();
    let mut ttls: Vec<u32> = Vec::new();

    for _ in 0..config.ping_count {
        let output = if is_windows {
            Command::new("ping")
                .args(["-n", "1", "-w", &config.timeout.to_string()])
                .arg(ping_target)
                .output()
        } else {
            Command::new("ping")
                .args(["-c", "1", "-W", &(config.timeout / 1000).to_string()])
                .arg(ping_target)
                .output()
        };

        match output {
            Ok(out) => {
                let stdout = String::from_utf8_lossy(&out.stdout);
                if let Some((ttl, rtt)) = parse_ping_output(&stdout, is_windows) {
                    rtts.push(rtt);
                    ttls.push(ttl);
                }
            }
            Err(_) => {}
        }
    }

    if rtts.is_empty() {
        return PingResult {
            ip: target.to_string(),
            raw_ip,
            status: "Timeout".to_string(),
            avg_rtt: 0.0,
            min_rtt: 0.0,
            max_rtt: 0.0,
            ttl: None,
            packet_loss: 100.0,
            timestamp,
            error_msg: None,
        };
    }

    let avg_rtt = rtts.iter().sum::<f64>() / rtts.len() as f64;
    let min_rtt = rtts.iter().cloned().fold(f64::INFINITY, f64::min);
    let max_rtt = rtts.iter().cloned().fold(f64::NEG_INFINITY, f64::max);
    let packet_loss = (config.ping_count - rtts.len()) as f64 / config.ping_count as f64 * 100.0;
    let ttl = ttls.last().copied();

    PingResult {
        ip: target.to_string(),
        raw_ip,
        status: "Online".to_string(),
        avg_rtt,
        min_rtt,
        max_rtt,
        ttl,
        packet_loss,
        timestamp,
        error_msg: None,
    }
}

fn chrono_lite_timestamp() -> String {
    use std::time::SystemTime;
    let now = SystemTime::now()
        .duration_since(SystemTime::UNIX_EPOCH)
        .unwrap_or_default();
    let secs = now.as_secs();
    let hours = (secs % 86400) / 3600;
    let mins = (secs % 3600) / 60;
    let s = secs % 60;
    format!("{:02}:{:02}:{:02}", hours, mins, s)
}

pub async fn ping_targets<F>(targets: Vec<String>, config: &PingConfig, stop_flag: &Arc<AtomicBool>, on_result: F) -> Vec<PingResult>
where
    F: Fn(PingResult, ProgressInfo) + Send + 'static,
{
    let total = targets.len();
    let sem = Arc::new(Semaphore::new(config.concurrency as usize));
    let is_windows = cfg!(windows);

    let mut handles = Vec::new();
    let mut results: Vec<PingResult> = Vec::with_capacity(total);

    for (idx, target) in targets.into_iter().enumerate() {
        if stop_flag.load(Ordering::SeqCst) {
            break;
        }

        let sem = Arc::clone(&sem);
        let stop_flag = Arc::clone(stop_flag);
        let config = PingConfig {
            timeout: config.timeout,
            packet_size: config.packet_size,
            interval: config.interval,
            concurrency: config.concurrency,
            ping_count: config.ping_count,
        };

        let handle = tokio::spawn(async move {
            let _permit = sem.acquire().await;

            if stop_flag.load(Ordering::SeqCst) {
                return None;
            }

            Some(ping_single(&target, &config, is_windows))
        });

        handles.push(handle);
    }

    for (idx, handle) in handles.into_iter().enumerate() {
        if let Ok(Some(result)) = handle.await {
            let online = if result.status == "Online" { 1 } else { 0 };
            let offline = if result.status != "Online" { 1 } else { 0 };

            let progress = ProgressInfo {
                total,
                completed: idx + 1,
                online,
                offline,
                percent: (idx + 1) as f64 / total as f64 * 100.0,
            };

            on_result(result.clone(), progress);
            results.push(result);
        }
    }

    results
}

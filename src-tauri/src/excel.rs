use crate::ping::PingResult;
use anyhow::Result;
use calamine::{open_workbook, Reader, Xlsx};
use regex::Regex;
use std::collections::HashSet;

static IP_REGEX: once_cell::sync::Lazy<Regex> =
    once_cell::sync::Lazy::new(|| Regex::new(r"\b(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\b").unwrap());

pub async fn import_from_file(path: &str) -> Result<Vec<String>> {
    let path_lower = path.to_lowercase();

    if path_lower.ends_with(".txt") {
        import_txt(path).await
    } else if path_lower.ends_with(".xlsx") || path_lower.ends_with(".xls") {
        import_excel(path).await
    } else {
        Err(anyhow::anyhow!("Unsupported file format"))
    }
}

async fn import_txt(path: &str) -> Result<Vec<String>> {
    let content = tokio::fs::read_to_string(path).await?;
    let mut ips = Vec::new();
    let mut seen = HashSet::new();

    for line in content.lines() {
        let line = line.trim();
        if line.is_empty() {
            continue;
        }

        let expanded = crate::ping::parse_input(line)?;
        for ip in expanded {
            if !seen.contains(&ip) {
                seen.insert(ip.clone());
                ips.push(ip);
            }
        }
    }

    Ok(ips)
}

async fn import_excel(path: &str) -> Result<Vec<String>> {
    let mut workbook: Xlsx<std::io::BufReader<std::fs::File>> = open_workbook(path)?;

    let sheets = workbook.sheet_names().to_vec();
    let mut all_ips = Vec::new();
    let mut seen = HashSet::new();

    for sheet_name in sheets {
        if let Ok(range) = workbook.worksheet_range(&sheet_name) {
            for row in range.rows() {
                for cell in row {
                    let cell_text = match cell {
                        calamine::Data::String(s) => s.clone(),
                        calamine::Data::Float(f) => {
                            if *f == f.floor() && *f >= 0.0 && *f <= 255.0 {
                                format!("{}", *f as u32)
                            } else {
                                continue;
                            }
                        }
                        _ => continue,
                    };

                    for cap in IP_REGEX.captures_iter(&cell_text) {
                        if let Some(ip) = cap.get(1) {
                            let ip_str = ip.as_str();
                            if is_valid_ip(ip_str) && !seen.contains(ip_str) {
                                seen.insert(ip_str.to_string());
                                all_ips.push(ip_str.to_string());
                            }
                        }
                    }
                }
            }
        }
    }

    Ok(all_ips)
}

fn is_valid_ip(s: &str) -> bool {
    let parts: Vec<&str> = s.split('.').collect();
    if parts.len() != 4 {
        return false;
    }
    parts.iter().all(|p| p.parse::<u8>().is_ok())
}

pub fn export_to_excel(path: &str, results: Vec<PingResult>) -> Result<()> {
    let mut workbook = xlsx_writer::Workbook::new();
    let sheet_name = "Results";
    workbook.add_worksheet(sheet_name)?;

    let headers = vec![
        "IP",
        "Raw IP",
        "Status",
        "Avg RTT (ms)",
        "Min RTT (ms)",
        "Max RTT (ms)",
        "TTL",
        "Packet Loss (%)",
        "Timestamp",
    ];

    for (col, header) in headers.iter().enumerate() {
        workbook.write_string(0, col as u16, header, None)?;
    }

    for (row, result) in results.iter().enumerate() {
        let row_idx = row as u32 + 1;
        workbook.write_string(row_idx, 0, &result.ip, None)?;
        workbook.write_string(row_idx, 1, result.raw_ip.as_deref().unwrap_or(""), None)?;
        workbook.write_string(row_idx, 2, &result.status, None)?;
        workbook.write_number(row_idx, 3, result.avg_rtt)?;
        workbook.write_number(row_idx, 4, result.min_rtt)?;
        workbook.write_number(row_idx, 5, result.max_rtt)?;
        workbook.write_number(row_idx, 6, result.ttl.unwrap_or(0) as f64)?;
        workbook.write_number(row_idx, 7, result.packet_loss)?;
        workbook.write_string(row_idx, 8, &result.timestamp, None)?;
    }

    workbook.save(path)?;
    Ok(())
}

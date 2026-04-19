use crate::config::{load_config, save_config};
use crate::excel::{export_to_excel, import_from_file};
use crate::ping::{parse_input, PingConfig, PingResult};
use crate::{get_stop_flag, reset_stop_flag, set_stop_flag};
use serde::{Deserialize, Serialize};
use tracing::info;

#[derive(Debug, Serialize, Deserialize)]
pub struct StartPingRequest {
    pub targets: Vec<String>,
    pub timeout: u32,
    pub packet_size: u32,
    pub interval: u32,
    pub concurrency: u32,
    pub ping_count: u32,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ProgressEvent {
    pub result: PingResult,
    pub progress: ProgressInfo,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ProgressInfo {
    pub total: usize,
    pub completed: usize,
    pub online: usize,
    pub offline: usize,
    pub percent: f64,
}

#[tauri::command]
pub async fn start_ping(
    app: tauri::AppHandle,
    request: StartPingRequest,
) -> Result<Vec<PingResult>, String> {
    info!("Starting ping with {} targets", request.targets.len());

    reset_stop_flag();
    let stop_flag = get_stop_flag();

    let config = PingConfig {
        timeout: request.timeout,
        packet_size: request.packet_size,
        interval: request.interval,
        concurrency: request.concurrency,
        ping_count: request.ping_count,
    };

    let results = ping::ping_targets(request.targets, &config, &stop_flag, move |result, progress| {
        let event = ProgressEvent {
            result,
            progress: progress.clone(),
        };
        if let Err(e) = app.emit("ping-progress", &event) {
            tracing::warn!("Failed to emit progress event: {}", e);
        }
    })
    .await;

    info!("Ping complete, {} results", results.len());
    Ok(results)
}

#[tauri::command]
pub fn stop_ping() -> Result<(), String> {
    info!("Stop ping requested");
    set_stop_flag();
    Ok(())
}

#[tauri::command]
pub fn parse_input(input: String) -> Result<Vec<String>, String> {
    parse_input(&input).map_err(|e| e.to_string())
}

#[tauri::command]
pub async fn import_file(path: String) -> Result<Vec<String>, String> {
    info!("Importing file: {}", path);
    import_from_file(&path).await.map_err(|e| e.to_string())
}

#[tauri::command]
pub fn export_excel(path: String, results: Vec<PingResult>) -> Result<(), String> {
    info!("Exporting {} results to {}", results.len(), path);
    export_to_excel(&path, results).map_err(|e| e.to_string())
}

#[tauri::command]
pub fn get_config() -> Result<config::Config, String> {
    load_config().map_err(|e| e.to_string())
}

#[tauri::command]
pub fn save_config(cfg: config::Config) -> Result<(), String> {
    info!("Saving config");
    save_config(&cfg).map_err(|e| e.to_string())
}

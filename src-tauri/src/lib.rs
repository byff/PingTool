mod commands;
mod config;
mod excel;
mod ping;

use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use tauri::Manager;
use tracing::{error, info};
use tracing_appender::rolling::{RollingFileAppender, Rotation};
use tracing_subscriber::{fmt, prelude::*, EnvFilter};

static STOP_FLAG: AtomicBool = AtomicBool::new(false);

pub fn get_stop_flag() -> Arc<AtomicBool> {
    Arc::new(STOP_FLAG)
}

pub fn reset_stop_flag() {
    STOP_FLAG.store(false, Ordering::SeqCst);
}

pub fn set_stop_flag() {
    STOP_FLAG.store(true, Ordering::SeqCst);
}

fn setup_logging() {
    let log_dir = dirs::data_local_dir()
        .unwrap_or_else(|| std::path::PathBuf::from("."))
        .join("PingTool")
        .join("logs");

    std::fs::create_dir_all(&log_dir).ok();

    let file_appender = RollingFileAppender::new(Rotation::DAILY, log_dir, "pingtool.log");
    let (non_blocking, _guard) = tracing_appender::non_blocking(file_appender);

    tracing_subscriber::registry()
        .with(EnvFilter::new("info"))
        .with(fmt::layer().with_writer(non_blocking).with_ansi(false))
        .with(fmt::layer().with_writer(std::io::stderr))
        .init();

    std::panic::set_hook(Box::new(|panic_info| {
        error!("PANIC: {}", panic_info);
    }));
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    setup_logging();
    info!("PingTool starting...");

    reset_stop_flag();

    tauri::Builder::default()
        .plugin(tauri_plugin_shell::init())
        .invoke_handler(tauri::generate_handler![
            commands::start_ping,
            commands::stop_ping,
            commands::parse_input,
            commands::import_file,
            commands::export_excel,
            commands::get_config,
            commands::save_config,
        ])
        .setup(|app| {
            info!("PingTool setup complete");
            Ok(())
        })
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

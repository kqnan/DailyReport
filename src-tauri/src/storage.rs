use chrono::Local;
use directories::ProjectDirs;
use std::fs::{self, File};
use std::io::{BufReader, BufWriter};
use std::path::PathBuf;
use tauri::AppHandle;

use crate::models::WorkSession;

const STORAGE_DIR: &str = "work_sessions";
const FILENAME_DATE_FORMAT: &str = "%Y-%m";

pub fn get_storage_dir(app_handle: &AppHandle) -> PathBuf {
    let proj_dirs = ProjectDirs::from("com", "daily-report", "DailyReport")
        .expect("Failed to get project directories");
    let mut path = proj_dirs.data_dir().to_path_buf();
    path.push(STORAGE_DIR);
    path
}

pub fn init_storage(app_handle: &AppHandle) -> Result<(), String> {
    let path = get_storage_dir(app_handle);
    fs::create_dir_all(&path).map_err(|e| format!("Failed to create storage dir: {}", e))?;
    Ok(())
}

pub fn get_filename_for_date(date: &str) -> String {
    // Parse YYYY-MM-DD to get YYYY-MM
    if let Some((year_month, _)) = date.split_once('-') {
        format!("sessions_{}.json", year_month.replace('-', "_"))
    } else {
        "sessions_unknown.json".to_string()
    }
}

pub fn load_sessions_for_date(app_handle: &AppHandle, date: &str) -> Vec<WorkSession> {
    let storage_path = get_storage_dir(app_handle);
    let filename = get_filename_for_date(date);
    let filepath = storage_path.join(&filename);

    if !filepath.exists() {
        return Vec::new();
    }

    match File::open(&filepath) {
        Ok(file) => {
            let reader = BufReader::new(file);
            match serde_json::from_reader(reader) {
                Ok(sessions) => sessions,
                Err(e) => {
                    eprintln!("Failed to parse {}: {}", filename, e);
                    Vec::new()
                }
            }
        }
        Err(e) => {
            eprintln!("Failed to open {}: {}", filename, e);
            Vec::new()
        }
    }
}

pub fn save_sessions(app_handle: &AppHandle, date: &str, sessions: &[WorkSession]) -> Result<(), String> {
    let storage_path = get_storage_dir(app_handle);
    let filename = get_filename_for_date(date);
    let filepath = storage_path.join(&filename);

    fs::create_dir_all(&storage_path).map_err(|e| format!("Failed to create dir: {}", e))?;

    let file = File::create(&filepath)
        .map_err(|e| format!("Failed to create file: {}", e))?;
    let writer = BufWriter::new(file);

    serde_json::to_writer_pretty(writer, sessions)
        .map_err(|e| format!("Failed to save sessions: {}", e))?;

    Ok(())
}

pub fn get_all_dates(app_handle: &AppHandle) -> Vec<String> {
    let storage_path = get_storage_dir(app_handle);
    let mut dates = std::collections::HashSet::new();

    if let Ok(entries) = fs::read_dir(&storage_path) {
        for entry in entries.flatten() {
            let filename = entry.file_name();
            let filename_str = filename.to_string_lossy();

            if filename_str.starts_with("sessions_") && filename_str.ends_with(".json") {
                // Extract date portion from filename: sessions_2026_03.json
                let date_part = filename_str.replace("sessions_", "").replace(".json", "");
                if let Some((year, rest)) = date_part.split_once('_') {
                    if let Some(month) = rest.split_once('_') {
                        // Get all days for this month
                        for day in 1..=31 {
                            dates.insert(format!("{}-{}-{:02}", year, month.0, day));
                        }
                    }
                }
            }
        }
    }

    let mut dates_vec: Vec<String> = dates.into_iter().collect();
    dates_vec.sort();
    dates_vec
}

pub fn add_session(app_handle: &AppHandle, session: &WorkSession) -> Result<(), String> {
    let date = &session.date;
    let mut sessions = load_sessions_for_date(app_handle, date);
    sessions.push(session.clone());
    save_sessions(app_handle, date, &sessions)
}

pub fn update_session(app_handle: &AppHandle, date: &str, updated_session: &WorkSession) -> Result<(), String> {
    let mut sessions = load_sessions_for_date(app_handle, date);
    if let Some(pos) = sessions.iter().position(|s| s.id == updated_session.id) {
        sessions[pos] = updated_session.clone();
        save_sessions(app_handle, date, &sessions)
    } else {
        Err("Session not found".to_string())
    }
}

pub fn get_today_sessions(app_handle: &AppHandle) -> Vec<WorkSession> {
    let today = Local::now().format("%Y-%m-%d").to_string();
    load_sessions_for_date(app_handle, &today)
}

pub fn get_active_session(app_handle: &AppHandle) -> Option<WorkSession> {
    get_today_sessions(app_handle)
        .into_iter()
        .find(|s| s.is_active())
}

pub fn delete_session(app_handle: &AppHandle, date: &str, session_id: &str) -> Result<(), String> {
    let mut sessions = load_sessions_for_date(app_handle, date);
    let initial_len = sessions.len();
    sessions.retain(|s| s.id != session_id);

    if sessions.len() == initial_len {
        return Err("Session not found".to_string());
    }

    save_sessions(app_handle, date, &sessions)
}

// 重命名端点函数
pub fn end_shift_session(app_handle: &AppHandle, date: &str, session_id: &str) -> Result<(), String> {
    let mut sessions = load_sessions_for_date(app_handle, date);
    if let Some(pos) = sessions.iter().position(|s| s.id == session_id) {
        let mut session = sessions.remove(pos);
        session.end_time = Some(chrono::Utc::now().to_rfc3339());
        session.duration_hours = session.calculate_duration();
        sessions.insert(pos, session);
        save_sessions(app_handle, date, &sessions)
    } else {
        Err("Session not found".to_string())
    }
}

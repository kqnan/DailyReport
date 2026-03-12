use crate::models::{DailyStatistics, WorkSession};
use crate::storage::{add_session, delete_session, end_shift, get_active_session, get_all_dates, get_today_sessions, load_sessions_for_date, save_sessions, update_session};
use crate::time_tracker::{end_shift as tt_end_shift, format_duration};
use tauri::Manager;

#[tauri::command]
pub fn start_shift(app: tauri::AppHandle) -> Result<WorkSession, String> {
    let mut session = crate::time_tracker::start_shift();

    // 检查是否已有进行中的记录
    if let Some(active) = get_active_session(&app) {
        return Err("已经有进行中的上班记录，请先下班".to_string());
    }

    // 添加默认工作类型
    session.work_type = "默认".to_string();
    session.activity = "工作片段".to_string();

    add_session(&app, &session).map_err(|e| format!("Failed to start shift: {}", e))?;
    Ok(session)
}

#[tauri::command]
pub fn end_shift(app: tauri::AppHandle, activity: String, work_type: String) -> Result<WorkSession, String> {
    let mut sessions = get_today_sessions(&app);
    let active_session = sessions.iter().find(|s| s.is_active());

    let session_id = match active_session {
        Some(s) => s.id.clone(),
        None => return Err("没有进行中的上班记录".to_string()),
    };

    // 找到并更新会话
    if let Some(pos) = sessions.iter().position(|s| s.id == session_id) {
        let mut session = sessions.remove(pos);
        session.activity = activity;
        session.work_type = work_type;

        tt_end_shift(&mut session);

        sessions.insert(pos, session.clone());
        save_sessions(&app, &session.date, &sessions).map_err(|e| format!("Failed to end shift: {}", e))?;

        Ok(session)
    } else {
        Err("Session not found".to_string())
    }
}

#[tauri::command]
pub fn get_sessions(app: tauri::AppHandle, date: String) -> Result<Vec<WorkSession>, String> {
    Ok(load_sessions_for_date(&app, &date))
}

#[tauri::command]
pub fn update_session_cmd(
    app: tauri::AppHandle,
    date: String,
    session_id: String,
    activity: String,
    work_type: String,
) -> Result<WorkSession, String> {
    let mut sessions = load_sessions_for_date(&app, &date);

    if let Some(pos) = sessions.iter().position(|s| s.id == session_id) {
        let mut session = sessions.remove(pos);
        session.activity = activity;
        session.work_type = work_type;

        // 重新计算时长（以防开始/结束时间被修改）
        session.duration_hours = session.calculate_duration();

        sessions.insert(pos, session.clone());
        save_sessions(&app, &date, &sessions).map_err(|e| format!("Failed to update session: {}", e))?;

        Ok(session)
    } else {
        Err("Session not found".to_string())
    }
}

#[tauri::command]
pub fn export_data(app: tauri::AppHandle, date: String, format: String) -> Result<String, String> {
    let sessions = load_sessions_for_date(&app, &date);

    match format.to_lowercase().as_str() {
        "json" => {
            serde_json::to_string_pretty(&sessions)
                .map_err(|e| format!("Failed to export JSON: {}", e))
        }
        "csv" => {
            let mut csv = "日期,开始时间,结束时间,时长(小时),工作类型,工作内容\n".to_string();
            for session in sessions {
                let end_time = session.end_time.unwrap_or_else(|| "进行中".to_string());
                let duration = format_duration(session.duration_hours);
                csv.push_str(&format!(
                    "{},{},{},{},{},\"{}\"\n",
                    session.date,
                    session.start_time,
                    end_time,
                    session.duration_hours,
                    session.work_type,
                    session.activity
                ));
            }
            Ok(csv)
        }
        _ => Err("Unsupported format. Use 'json' or 'csv'".to_string()),
    }
}

#[tauri::command]
pub fn get_today_statistics(app: tauri::AppHandle) -> Result<DailyStatistics, String> {
    let today = chrono::Local::now().format("%Y-%m-%d").to_string();
    let sessions = get_today_sessions(&app);

    let total_hours = sessions.iter().map(|s| s.duration_hours).sum();
    let session_count = sessions.len();

    Ok(DailyStatistics {
        date: today,
        total_hours,
        session_count,
    })
}

#[tauri::command]
pub fn get_available_dates(app: tauri::AppHandle) -> Result<Vec<String>, String> {
    Ok(get_all_dates(&app))
}

#[tauri::command]
pub fn delete_session_cmd(app: tauri::AppHandle, date: String, session_id: String) -> Result<(), String> {
    delete_session(&app, &date, &session_id)
}

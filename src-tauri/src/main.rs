use tauri::{Manager, Runtime};
use tauri_plugin_traymenu::TrayMenuBuilder;
use tauri_plugin_window_state::WindowPositionStateExt;

mod commands;
mod models;
mod storage;
mod time_tracker;

fn main() {
    tauri::Builder::default()
        .plugin(tauri_plugin_window_state::Builder::default().build())
        .plugin(tauri_plugin_traymenu::init())
        .setup(|app| {
            let handle = app.handle();
            tauri::async_runtime::spawn(async move {
                // 初始化存储目录
                let _ = storage::init_storage(&handle);
            });

            // 创建系统托盘菜单
            let tray_menu = TrayMenuBuilder::new(app)
                .menu_item("显示主窗口")
                .separator()
                .menu_item("退出")
                .build()
                .expect("Failed to build tray menu");

            // 设置托盘图标点击事件
            tray_menu.on_event(move |event| {
                if let tauri_plugin_traymenu::TrayMenuEvent::MenuItemClick { id } = event {
                    match id.as_str() {
                        "显示主窗口" => {
                            let window = app.get_webview_window("main").expect("No main window");
                            window.show().expect("Failed to show window");
                            window.set_focus().expect("Failed to focus window");
                        }
                        "退出" => {
                            // 结束进行中的记录
                            let sessions = storage::get_today_sessions(app);
                            if let Some(active_session) = sessions.iter().find(|s| s.is_active()) {
                                let date = active_session.date.clone();
                                let session_id = active_session.id.clone();
                                let _ = storage::end_shift_session(app, &date, &session_id);
                            }
                            app.exit(0);
                        }
                        _ => {}
                    }
                }
            });

            // 处理窗口关闭事件
            app.handle().on_window_close(|window| {
                let app_handle = window.app_handle();
                // 检查是否有进行中的记录
                let sessions = storage::get_today_sessions(&app_handle);
                if let Some(active_session) = sessions.iter().find(|s| s.is_active()) {
                    let date = active_session.date.clone();
                    let session_id = active_session.id.clone();
                    let _ = storage::end_shift_session(&app_handle, &date, &session_id);
                }
                Ok(())
            });

            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            commands::start_shift,
            commands::end_shift,
            commands::get_sessions,
            commands::update_session_cmd,
            commands::delete_session_cmd,
            commands::export_data,
            commands::get_today_statistics,
            commands::get_available_dates
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

// Unit tests for models and time_tracker modules

#[path = "../src/models.rs"]
mod models;

#[path = "../src/time_tracker.rs"]
mod time_tracker;

#[test]
fn test_work_session_creation() {
    let now = chrono::Utc::now();
    let date = now.format("%Y-%m-%d").to_string();
    let start_time = now.to_rfc3339();

    let session = models::WorkSession::new(start_time.clone(), date.clone());

    assert_eq!(session.date, date);
    assert_eq!(session.start_time, start_time);
    assert!(session.end_time.is_none());
    assert_eq!(session.duration_hours, 0.0);
    // end_time is None, so is_active() should return true
    assert!(session.is_active());
}

#[test]
fn test_work_session_duration_calculation() {
    let start_time = "2026-03-11T09:00:00Z".to_string();
    let end_time = "2026-03-11T10:30:00Z".to_string();
    let date = "2026-03-11".to_string();

    let mut session = models::WorkSession::new(start_time, date);
    session.end_time = Some(end_time);

    // 1.5小时应该舍入到1.5
    assert_eq!(session.calculate_duration(), 1.5);
}

#[test]
fn test_work_session_duration_rounding() {
    let start_time = "2026-03-11T09:00:00Z".to_string();
    // 1小时35分钟 = 1.5833...小时，应该舍入到1.6
    let end_time = "2026-03-11T10:35:00Z".to_string();
    let date = "2026-03-11".to_string();

    let mut session = models::WorkSession::new(start_time, date);
    session.end_time = Some(end_time);

    assert_eq!(session.calculate_duration(), 1.6);
}

#[test]
fn test_work_session_is_active() {
    let now = chrono::Utc::now();
    let date = now.format("%Y-%m-%d").to_string();
    let start_time = now.to_rfc3339();

    // 未结束的会话
    let session = models::WorkSession::new(start_time.clone(), date.clone());
    assert!(session.is_active());

    // 已结束的会话
    let mut ended_session = models::WorkSession::new(start_time, date);
    ended_session.end_time = Some("2026-03-11T10:00:00Z".to_string());
    assert!(!ended_session.is_active());
}

#[test]
fn test_work_session_exact_hour() {
    let start_time = "2026-03-11T09:00:00Z".to_string();
    let end_time = "2026-03-11T11:00:00Z".to_string();
    let date = "2026-03-11".to_string();

    let mut session = models::WorkSession::new(start_time, date);
    session.end_time = Some(end_time);

    assert_eq!(session.calculate_duration(), 2.0);
}

#[test]
fn test_work_session_minutes_only() {
    let start_time = "2026-03-11T09:00:00Z".to_string();
    let end_time = "2026-03-11T09:45:00Z".to_string();
    let date = "2026-03-11".to_string();

    let mut session = models::WorkSession::new(start_time, date);
    session.end_time = Some(end_time);

    // 45分钟 = 0.75小时，应该舍入到0.8
    assert_eq!(session.calculate_duration(), 0.8);
}

#[test]
fn test_start_shift_creates_session() {
    let session = time_tracker::start_shift();

    let now = chrono::Utc::now();
    let today = now.format("%Y-%m-%d").to_string();

    assert_eq!(session.date, today);
    assert!(session.end_time.is_none());
    assert!(session.is_active());
}

#[test]
fn test_end_shift_sets_end_time() {
    let mut session = time_tracker::start_shift();

    // 稍微等待一下确保时间有变化
    std::thread::sleep(std::time::Duration::from_millis(100));

    let duration = time_tracker::end_shift(&mut session);

    assert!(session.end_time.is_some());
    // 由于时长四舍五入到小数点后一位，短时间可能为0.0，所以只检查end_time被设置
    // 或者检查duration >= 0
    assert!(duration >= 0.0);
}

#[test]
fn test_format_duration_hours() {
    assert_eq!(time_tracker::format_duration(2.0), "2小时");
    assert_eq!(time_tracker::format_duration(1.0), "1小时");
}

#[test]
fn test_format_duration_minutes() {
    assert_eq!(time_tracker::format_duration(0.5), "30分钟");
    assert_eq!(time_tracker::format_duration(0.25), "15分钟");
}

#[test]
fn test_format_duration_mixed() {
    assert_eq!(time_tracker::format_duration(1.5), "1小时30分钟");
    assert_eq!(time_tracker::format_duration(2.25), "2小时15分钟");
}

#[test]
fn test_format_duration_zero() {
    assert_eq!(time_tracker::format_duration(0.0), "0分钟");
}

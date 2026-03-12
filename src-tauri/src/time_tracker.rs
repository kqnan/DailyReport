use crate::models::WorkSession;
use chrono::{DateTime, Utc};

pub fn start_shift() -> WorkSession {
    let now = Utc::now();
    let date = now.format("%Y-%m-%d").to_string();
    let start_time = now.to_rfc3339();

    WorkSession::new(start_time, date)
}

pub fn end_shift(session: &mut WorkSession) -> f64 {
    let now = Utc::now();
    session.end_time = Some(now.to_rfc3339());
    session.duration_hours = session.calculate_duration();
    session.duration_hours
}

pub fn calculate_duration_from_range(start: &str, end: &str) -> f64 {
    let start_dt = DateTime::parse_from_rfc3339(start)
        .unwrap()
        .with_timezone(&Utc);
    let end_dt = DateTime::parse_from_rfc3339(end)
        .unwrap()
        .with_timezone(&Utc);
    let duration = end_dt.timestamp() - start_dt.timestamp();
    // 精确到小数点后一位
    (duration as f64 / 3600.0 * 10.0).round() / 10.0
}

pub fn format_duration(hours: f64) -> String {
    let h = hours as i32;
    let m = ((hours - h as f64) * 60.0).round() as i32;

    if h == 0 {
        format!("{}分钟", m)
    } else if m == 0 {
        format!("{}小时", h)
    } else {
        format!("{}小时{}分钟", h, m)
    }
}

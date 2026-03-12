use serde::{Deserialize, Serialize};
use uuid::Uuid;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct WorkSession {
    pub id: String,
    pub date: String,       // YYYY-MM-DD
    pub start_time: String, // ISO 8601 format
    pub end_time: Option<String>, // ISO 8601 format, null if still working
    pub duration_hours: f64,
    pub activity: String,
    pub work_type: String,
}

impl WorkSession {
    pub fn new(start_time: String, date: String) -> Self {
        Self {
            id: Uuid::new_v4().to_string(),
            date,
            start_time,
            end_time: None,
            duration_hours: 0.0,
            activity: "工作片段".to_string(),
            work_type: "默认".to_string(),
        }
    }

    pub fn calculate_duration(&self) -> f64 {
        if let Some(end_time) = &self.end_time {
            let start = chrono::DateTime::parse_from_rfc3339(&self.start_time)
                .unwrap()
                .with_timezone(&chrono::Utc);
            let end = chrono::DateTime::parse_from_rfc3339(end_time)
                .unwrap()
                .with_timezone(&chrono::Utc);
            let duration = end.timestamp() - start.timestamp();
            // 精确到小数点后一位
            (duration as f64 / 3600.0 * 10.0).round() / 10.0
        } else {
            0.0
        }
    }

    pub fn is_active(&self) -> bool {
        self.end_time.is_none()
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DailyStatistics {
    pub date: String,
    pub total_hours: f64,
    pub session_count: usize,
}

// WorkSession interface
export interface WorkSession {
  id: string;
  date: string;
  start_time: string;
  end_time: string | null;
  duration_hours: number;
  activity: string;
  work_type: string;
}

// DailyStatistics interface
export interface DailyStatistics {
  date: string;
  total_hours: number;
  session_count: number;
}

// 格式化时长显示
export function formatDuration(hours: number): string {
  const h = Math.floor(hours);
  const m = Math.round((hours - h) * 60);

  if (h === 0) {
    return `${m}分钟`;
  } else if (m === 0) {
    return `${h}小时`;
  } else {
    return `${h}小时${m}分钟`;
  }
}

declare global {
  interface Window {
    api: {
      startShift: () => Promise<WorkSession>;
      endShift: (activity: string, workType: string) => Promise<WorkSession>;
      getSessions: (date: string) => Promise<WorkSession[]>;
      updateSession: (date: string, sessionId: string, activity: string, workType: string) => Promise<WorkSession>;
      deleteSession: (date: string, sessionId: string) => Promise<void>;
      exportData: (date: string, format: string) => Promise<string>;
      getTodayStatistics: () => Promise<DailyStatistics>;
      getAvailableDates: () => Promise<string[]>;
    };
  }
}

export {};

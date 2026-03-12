import { WorkSession, DailyStatistics } from './utils';

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

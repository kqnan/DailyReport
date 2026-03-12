import React, { useState, useEffect } from 'react';
import './App.css';
import { formatDuration } from './utils';

interface WorkSession {
  id: string;
  date: string;
  start_time: string;
  end_time: string | null;
  duration_hours: number;
  activity: string;
  work_type: string;
}

interface DailyStatistics {
  date: string;
  total_hours: number;
  session_count: number;
}

function App() {
  const [activeSession, setActiveSession] = useState<WorkSession | null>(null);
  const [sessions, setSessions] = useState<WorkSession[]>([]);
  const [selectedDate, setSelectedDate] = useState<string>(new Date().toISOString().split('T')[0]);
  const [isEndDialogOpen, setIsEndDialogOpen] = useState(false);
  const [activity, setActivity] = useState('');
  const [workType, setWorkType] = useState('工作片段');
  const [editableSession, setEditableSession] = useState<WorkSession | null>(null);
  const [editDialogOpen, setEditDialogOpen] = useState(false);
  const [statistics, setStatistics] = useState<DailyStatistics | null>(null);
  const [availableDates, setAvailableDates] = useState<string[]>([]);
  const [error, setError] = useState<string | null>(null);

  // 加载今天的记录
  useEffect(() => {
    loadTodaySessions();
    loadAvailableDates();
  }, []);

  // 加载选定日期的记录
  useEffect(() => {
    if (selectedDate) {
      loadSessions(selectedDate);
    }
  }, [selectedDate]);

  const loadTodaySessions = async () => {
    try {
      const response = await window.api.getTodayStatistics();
      if (response) {
        setStatistics(response);
      }
    } catch (e) {
      // API 还没准备好
    }

    // 检查是否有进行中的记录
    const todaySessions = sessions.filter(s => s.date === new Date().toISOString().split('T')[0] && !s.end_time);
    if (todaySessions.length > 0) {
      setActiveSession(todaySessions[0]);
    }
  };

  const loadAvailableDates = async () => {
    try {
      const response = await window.api.getAvailableDates();
      if (Array.isArray(response)) {
        setAvailableDates(response);
      }
    } catch (e) {
      console.error('加载日期失败:', e);
    }
  };

  const loadSessions = async (date: string) => {
    try {
      const response = await window.api.getSessions(date);
      if (Array.isArray(response)) {
        setSessions(response.sort((a, b) => a.start_time.localeCompare(b.start_time)));
      }
    } catch (e) {
      console.error('加载记录失败:', e);
    }
  };

  const handleStartShift = async () => {
    try {
      const response = await window.api.startShift();
      if (response) {
        setActiveSession(response);
        setSessions(prev => [...prev, response]);
        setError(null);
      }
    } catch (e: any) {
      setError(e.message || '开始上班失败');
    }
  };

  const handleEndShift = () => {
    setActivity('工作片段');
    setWorkType('工作片段');
    setIsEndDialogOpen(true);
  };

  const confirmEndShift = async () => {
    if (!activeSession) return;

    try {
      const response = await window.api.endShift(activity || '无', workType);
      if (response) {
        setActiveSession(null);
        setSessions(prev => prev.filter(s => s.id !== activeSession.id).concat(response));
        setIsEndDialogOpen(false);
        setActivity('工作片段');
        setWorkType('工作片段');
        setError(null);
      }
    } catch (e: any) {
      setError(e.message || '结束上班失败');
    }
  };

  const handleEditSession = (session: WorkSession) => {
    setEditableSession(session);
    setActivity(session.activity);
    setWorkType(session.work_type);
    setEditDialogOpen(true);
  };

  const confirmEditSession = async () => {
    if (!editableSession) return;

    try {
      const response = await window.api.updateSession(editableSession.date, editableSession.id, activity, workType);
      if (response) {
        setSessions(prev => prev.map(s => s.id === response.id ? response : s));
        if (activeSession && activeSession.id === response.id) {
          setActiveSession(response);
        }
        setEditDialogOpen(false);
        setEditableSession(null);
      }
    } catch (e: any) {
      setError(e.message || '更新记录失败');
    }
  };

  const handleDeleteSession = async (session: WorkSession) => {
    if (window.confirm(`确定要删除 ${session.date} ${session.start_time.slice(11, 16)} 的记录吗？`)) {
      try {
        await window.api.deleteSession(session.date, session.id);
        setSessions(prev => prev.filter(s => s.id !== session.id));
        if (activeSession && activeSession.id === session.id) {
          setActiveSession(null);
        }
      } catch (e: any) {
        setError(e.message || '删除记录失败');
      }
    }
  };

  const handleExport = async (format: 'json' | 'csv') => {
    try {
      const data = await window.api.exportData(selectedDate, format);
      const blob = new Blob([data], { type: format === 'json' ? 'application/json' : 'text/csv' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `工时记录_${selectedDate}.${format === 'json' ? 'json' : 'csv'}`;
      a.click();
      URL.revokeObjectURL(url);
    } catch (e: any) {
      setError(e.message || '导出失败');
    }
  };

  const filteredSessions = sessions.filter(s => s.date === selectedDate);

  return (
    <div className="app-container">
      <header className="app-header">
        <h1>工时记录</h1>
      </header>

      {error && (
        <div className="error-alert">
          {error}
          <button onClick={() => setError(null)} className="close-btn">×</button>
        </div>
      )}

      {/* 今日统计卡片 */}
      {statistics && (
        <div className="stat-card">
          <div className="stat-item">
            <span className="stat-label">今日已工作</span>
            <span className="stat-value">{formatDuration(statistics.total_hours)}</span>
          </div>
          <div className="stat-item">
            <span className="stat-label">上班片段</span>
            <span className="stat-value">{statistics.session_count} 段</span>
          </div>
        </div>
      )}

      {/* 上班/下班按钮 */}
      <div className="control-card">
        {activeSession ? (
          <>
            <h2 className="status-working">当前状态：工作中</h2>
            <p className="start-time">
              开始时间：{activeSession.start_time.slice(0, 16).replace('T', ' ')}
            </p>
            <button className="btn-end" onClick={handleEndShift}>
              💻 下班
            </button>
          </>
        ) : (
          <button className="btn-start" onClick={handleStartShift}>
            🟢 上班
          </button>
        )}
      </div>

      {/* 日期选择器 */}
      <div className="date-selector">
        <label>选择日期：</label>
        <input
          type="date"
          value={selectedDate}
          onChange={(e) => setSelectedDate(e.target.value)}
        />
        <button onClick={() => setSelectedDate(new Date().toISOString().split('T')[0])}>
          今天
        </button>
      </div>

      {/* 导出按钮 */}
      <div className="export-buttons">
        <button onClick={() => handleExport('csv')}>📥 导出 CSV</button>
        <button onClick={() => handleExport('json')}>📥 导出 JSON</button>
      </div>

      {/* 记录列表 */}
      <div className="sessions-container">
        <h2>{selectedDate} 的记录 ({filteredSessions.length} 段)</h2>

        {filteredSessions.length === 0 ? (
          <div className="empty-message">
            暂无记录
            <p>点击上方的"上班"按钮开始记录</p>
          </div>
        ) : (
          <ul className="session-list">
            {filteredSessions
              .sort((a, b) => a.start_time.localeCompare(b.start_time))
              .map((session) => (
                <li key={session.id} className="session-item">
                  <div className="session-header">
                    <span className="time-range">
                      {session.start_time.slice(11, 16)} - {session.end_time ? session.end_time.slice(11, 16) : '进行中'}
                    </span>
                    <span className={`duration-badge ${session.end_time ? '' : 'active'}`}>
                      {formatDuration(session.duration_hours)}
                    </span>
                  </div>
                  <div className="session-info">
                    <div className="work-type">类型：{session.work_type}</div>
                    <div className="work-activity">内容：{session.activity !== '工作片段' ? session.activity : '无描述'}</div>
                  </div>
                  <div className="session-actions">
                    <button onClick={() => handleEditSession(session)}>编辑</button>
                    <button onClick={() => handleDeleteSession(session)} className="btn-delete">删除</button>
                  </div>
                </li>
              ))}
          </ul>
        )}
      </div>

      {/* 下班对话框 */}
      {isEndDialogOpen && (
        <div className="modal-overlay" onClick={() => setIsEndDialogOpen(false)}>
          <div className="modal" onClick={(e) => e.stopPropagation()}>
            <h3>结束上班记录</h3>
            <div className="form-group">
              <label>工作内容：</label>
              <textarea
                value={activity}
                onChange={(e) => setActivity(e.target.value)}
                placeholder="描述一下今天做了什么..."
                rows={3}
              />
            </div>
            <div className="form-group">
              <label>工作类型：</label>
              <select value={workType} onChange={(e) => setWorkType(e.target.value)}>
                <option value="开发">开发</option>
                <option value="会议">会议</option>
                <option value="调试">调试</option>
                <option value="文档">文档</option>
                <option value="其他">其他</option>
              </select>
            </div>
            <div className="modal-actions">
              <button onClick={() => setIsEndDialogOpen(false)} className="btn-cancel">取消</button>
              <button onClick={confirmEndShift} className="btn-confirm">确认下班</button>
            </div>
          </div>
        </div>
      )}

      {/* 编辑对话框 */}
      {editDialogOpen && editableSession && (
        <div className="modal-overlay" onClick={() => setEditDialogOpen(false)}>
          <div className="modal" onClick={(e) => e.stopPropagation()}>
            <h3>编辑记录</h3>
            <div className="form-group">
              <label>开始时间：</label>
              <input
                type="datetime-local"
                value={editableSession.start_time.replace('Z', '').replace('T', ' ')}
                disabled
              />
            </div>
            {editableSession.end_time && (
              <div className="form-group">
                <label>结束时间：</label>
                <input
                  type="datetime-local"
                  value={editableSession.end_time.replace('Z', '').replace('T', ' ')}
                  disabled
                />
              </div>
            )}
            <div className="form-group">
              <label>工作内容：</label>
              <textarea
                value={activity}
                onChange={(e) => setActivity(e.target.value)}
                rows={3}
              />
            </div>
            <div className="form-group">
              <label>工作类型：</label>
              <select value={workType} onChange={(e) => setWorkType(e.target.value)}>
                <option value="开发">开发</option>
                <option value="会议">会议</option>
                <option value="调试">调试</option>
                <option value="文档">文档</option>
                <option value="其他">其他</option>
              </select>
            </div>
            <div className="modal-actions">
              <button onClick={() => setEditDialogOpen(false)} className="btn-cancel">取消</button>
              <button onClick={confirmEditSession} className="btn-confirm">保存</button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}

export default App;

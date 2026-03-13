#pragma once
#include "datamodel.h"
#include <QString>
#include <QMap>
#include <QJsonArray>
#include <QList>
#include <QPair>
#include <QObject>
#include <QString>

// Cloud work session record
struct CloudWorkRecord {
    QString uuid;              // Record UUID from server
    QString date;              // YYYY-MM-DD
    QString taskDescription;   // Work content
    double workingHours;       // Hours spent
    bool isSynced;             // Whether this record is synced to server
    QString endTime;           // End time if session ended

    // Convert WorkSession to CloudWorkRecord
    static CloudWorkRecord fromWorkSession(const WorkSession& session);
};

class CloudSessionManager : public QObject {
    Q_OBJECT

public:
    static CloudSessionManager& instance();

    // Set applicant info from login
    void setApplicantInfo(const QString& applicantId, const QString& applicantName);

    // Set today's daily report info
    void setTodayDailyReport(const QString& uuid, const QString& month, const QString& week);

    // Load sessions from server for today
    void loadTodaySessions();

    // Load sessions from server for a specific date
    void loadDailyReportDetails(const QString& date);

    // Load recent days' sessions from server
    void loadRecentDaysSessions(int days = 3);

    // Get session count for a date
    int getSessionCount(const QString& date) const;

    // Add session to buffer (内存中)
    void addSession(const WorkSession& session);

    // Update session in buffer
    void updateSession(const WorkSession& session);

    // Delete session from buffer
    void deleteSession(const QString& sessionId);

    // Sync today's sessions to server
    void syncToday();

    // Sync daily report with specific UUID
    void syncDailyReport(const QString& uuid, const QString& date);

    // Create today's daily report if not exists
    void createTodayDailyReportIfNotExist();

    // Get sessions for a date from buffer
    QList<WorkSession> getSessions(const QString& date) const;

    // Get today's sessions
    QList<WorkSession> getTodaySessions() const;

    // Get active session for a date
    WorkSession getActiveSessionForDate(const QString& date);

    // Get statistics for a date
    DailyStatistics getStatisticsForDate(const QString& date) const;

    // Get today's statistics
    DailyStatistics getTodayStatistics() const;

    // Parse daily report details from server API response
    void parseDailyReportDetails(const QJsonArray& tasks, const QString& date);

    // Get buffer (for testing)
    const QMap<QString, QList<CloudWorkRecord>>& getBuffer() const;

    // Clear buffer (for testing)
    void clearBuffer();

    // Get today's month and week for sync
    QString getTodayMonth() const { return todayMonth; }
    QString getTodayWeek() const { return todayWeek; }

private:
    CloudSessionManager() = default;

    QMap<QString, QList<CloudWorkRecord>> buffer;  // buffer[date] = [records]
    QString applicantId = "SQ13793";
    QString applicantName = "孔启楠";
    QString todayDailyReportUuid;
    QString todayMonth;
    QString todayWeek;

    QString getCurrentDate() const;
    QString getCurrentDayOfWeek(const QString& date) const;
};

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

    // Add session to buffer (内存中)
    void addSession(const WorkSession& session);

    // Update session in buffer
    void updateSession(const WorkSession& session);

    // Delete session from buffer
    void deleteSession(const QString& sessionId);

    // Sync today's sessions to server
    void syncToday();

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
    void parseDailyReportDetails(const QJsonArray& tasks);

    // Get buffer (for testing)
    const QMap<QString, QList<CloudWorkRecord>>& getBuffer() const;

private:
    CloudSessionManager() = default;

    QMap<QString, QList<CloudWorkRecord>> buffer;  // buffer[date] = [records]
    QString applicantId = "SQ13793";
    QString applicantName = "孔启楠";
    QString todayDailyReportUuid;
    QString todayMonth;
    QString todayWeek;

    QString getCurrentDate() const;
};

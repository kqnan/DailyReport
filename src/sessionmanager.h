#pragma once
#include "datamodel.h"
#include <QString>
#include <QList>
#include <QDir>

class SessionManager {
public:
    static SessionManager& instance();

    QList<WorkSession> loadSessions(const QString &date);
    void saveSessions(const QString &date, const QList<WorkSession> &sessions);
    void addSession(const WorkSession &session);
    void updateSession(const QString &date, const WorkSession &session);
    void deleteSession(const QString &date, const QString &sessionId);
    QList<QString> getAvailableDates();
    QList<WorkSession> getTodaySessions();
    WorkSession getActiveSession();
    WorkSession getActiveSessionForDate(const QString &date);
    DailyStatistics getTodayStatistics();
    DailyStatistics getStatisticsForDate(const QString &date);

    // Testing methods
    QString getFilenameForDate(const QString &date) const;

private:
    SessionManager() = default;

    QString getStorageDir() const;
    QList<WorkSession> loadFromFile(const QString &filepath);
    void saveToFile(const QString &filepath, const QList<WorkSession> &sessions);
};

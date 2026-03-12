#include "sessionmanager.h"
#include "utils.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QDateTime>
#include <QUuid>
#include <algorithm>

SessionManager& SessionManager::instance() {
    static SessionManager instance;
    return instance;
}

QString SessionManager::getStorageDir() const {
    return getStorageDirectory();
}

QString SessionManager::getFilenameForDate(const QString &date) const {
    // date: "2026-03-12" -> "sessions_2026_03.json"
    QString yearMonth = date.left(7).replace("-", "_");
    return QString("sessions_%1.json").arg(yearMonth);
}

QList<WorkSession> SessionManager::loadSessions(const QString &date) {
    QString filename = getFilenameForDate(date);
    QString filepath = getStorageDir() + "/" + filename;
    return loadFromFile(filepath);
}

QList<WorkSession> SessionManager::loadFromFile(const QString &filepath) {
    QList<WorkSession> sessions;
    QFile file(filepath);

    if (!file.exists()) return sessions;
    if (!file.open(QIODevice::ReadOnly)) return sessions;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isArray()) return sessions;

    QJsonArray array = doc.array();
    for (QJsonValue val : array) {
        if (!val.isObject()) continue;

        QJsonObject obj = val.toObject();
        WorkSession session;
        session.id = obj["id"].toString();
        session.date = obj["date"].toString();
        session.startTime = obj["startTime"].toString();
        session.endTime = obj["endTime"].toString();
        session.durationHours = obj["durationHours"].toDouble();
        session.activity = obj["activity"].toString();
        session.workType = obj["workType"].toString();

        sessions.append(session);
    }

    return sessions;
}

void SessionManager::saveToFile(const QString &filepath, const QList<WorkSession> &sessions) {
    QDir dir(QFileInfo(filepath).path());
    dir.mkpath(".");

    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) return;

    QJsonArray array;
    for (const WorkSession &session : sessions) {
        QJsonObject obj;
        obj["id"] = session.id;
        obj["date"] = session.date;
        obj["startTime"] = session.startTime;
        obj["endTime"] = session.endTime;
        obj["durationHours"] = session.durationHours;
        obj["activity"] = session.activity;
        obj["workType"] = session.workType;
        array.append(obj);
    }

    QJsonDocument doc(array);
    file.write(doc.toJson(QJsonDocument::Indented));
}

void SessionManager::saveSessions(const QString &date, const QList<WorkSession> &sessions) {
    QString filename = getFilenameForDate(date);
    QString filepath = getStorageDir() + "/" + filename;
    saveToFile(filepath, sessions);
}

void SessionManager::addSession(const WorkSession &session) {
    QList<WorkSession> sessions = loadSessions(session.date);
    sessions.append(session);
    saveSessions(session.date, sessions);
}

void SessionManager::updateSession(const QString &date, const WorkSession &session) {
    QList<WorkSession> sessions = loadSessions(date);
    for (int i = 0; i < sessions.size(); ++i) {
        if (sessions[i].id == session.id) {
            sessions[i] = session;
            break;
        }
    }
    saveSessions(date, sessions);
}

void SessionManager::deleteSession(const QString &date, const QString &sessionId) {
    QList<WorkSession> sessions = loadSessions(date);
    sessions.erase(std::remove_if(sessions.begin(), sessions.end(),
        [sessionId](const WorkSession &s) { return s.id == sessionId; }), sessions.end());
    saveSessions(date, sessions);
}

QList<QString> SessionManager::getAvailableDates() {
    QDir dir(getStorageDir());
    QList<QString> dates;

    QFileInfoList files = dir.entryInfoList(QStringList("sessions_*.json"), QDir::Files);
    for (const QFileInfo &info : files) {
        QString filename = info.fileName();
        // Extract date from "sessions_2026_03.json"
        QString datePart = filename.remove("sessions_").remove(".json").replace("_", "-");
        // Generate all dates for this month
        QString year = datePart.left(4);
        QString month = datePart.mid(5, 2);
        for (int day = 1; day <= 31; ++day) {
            dates.append(QString("%1-%2-%3").arg(year).arg(month).arg(day, 2, 10, QChar('0')));
        }
    }

    std::sort(dates.begin(), dates.end());
    return dates;
}

QList<WorkSession> SessionManager::getTodaySessions() {
    return loadSessions(getCurrentDate());
}

WorkSession SessionManager::getActiveSession() {
    QList<WorkSession> sessions = getTodaySessions();
    for (const WorkSession &s : sessions) {
        if (s.isActive()) return s;
    }
    return WorkSession();
}

DailyStatistics SessionManager::getTodayStatistics() {
    QList<WorkSession> sessions = getTodaySessions();
    DailyStatistics stat;
    stat.date = getCurrentDate();
    stat.totalHours = 0;
    stat.sessionCount = sessions.size();

    for (const WorkSession &s : sessions) {
        stat.totalHours += s.durationHours;
    }

    return stat;
}

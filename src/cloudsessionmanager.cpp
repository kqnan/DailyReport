#include "cloudsessionmanager.h"
#include "apimanager.h"
#include "utils.h"
#include <QApplication>
#include <QDebug>
#include <QDate>

CloudWorkRecord CloudWorkRecord::fromWorkSession(const WorkSession& session) {
    CloudWorkRecord record;
    record.uuid = session.id;
    record.date = session.date;
    record.taskDescription = session.activity;
    record.workingHours = session.durationHours;
    record.isSynced = false;
    return record;
}

CloudSessionManager& CloudSessionManager::instance() {
    static CloudSessionManager instance;
    return instance;
}

QString CloudSessionManager::getCurrentDate() const {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd");
}

int CloudSessionManager::getSessionCount(const QString& date) const {
    if (!buffer.contains(date)) return 0;
    return buffer[date].size();
}

void CloudSessionManager::setApplicantInfo(const QString& applicantId, const QString& applicantName) {
    this->applicantId = applicantId;
    this->applicantName = applicantName;
}

void CloudSessionManager::setTodayDailyReport(const QString& uuid, const QString& month, const QString& week) {
    todayDailyReportUuid = uuid;
    todayMonth = month;
    todayWeek = week;
    qDebug() << "设置今日日报: uuid=" << uuid << " month=" << month << " week=" << week;
}

void CloudSessionManager::loadTodaySessions() {
    QString today = getCurrentDate();
    if (todayDailyReportUuid.isEmpty()) {
        qDebug() << "今日日报UUID未设置，尝试创建日报...";
        // Try to create today's daily report if it doesn't exist
        createTodayDailyReportIfNotExist();
        return;
    }

    qDebug() << "正在获取今日日报详情...";
    ApiManager::instance().getDailyReportDetails(today, applicantId);
}

void CloudSessionManager::loadDailyReportDetails(const QString& date) {
    qDebug() << "正在获取" << date << "的日报详情...";
    ApiManager::instance().getDailyReportDetails(date, applicantId);
}

void CloudSessionManager::loadRecentDaysSessions(int days) {
    qDebug() << "正在加载最近" << days << "天的日报详情...";

    QDate today = QDate::currentDate();
    for (int i = 0; i < days; ++i) {
        QDate date = today.addDays(-i);
        QString dateStr = date.toString("yyyy-MM-dd");
        // Only load if not already loaded
        if (!buffer.contains(dateStr) || buffer[dateStr].isEmpty()) {
            qDebug() << "加载" << dateStr << "的日报详情";
            ApiManager::instance().getDailyReportDetails(dateStr, applicantId);
        }
    }
}

void CloudSessionManager::createTodayDailyReportIfNotExist() {
    QString today = getCurrentDate();
    if (todayDailyReportUuid.isEmpty()) {
        qDebug() << "创建今日日报(UUID: " << todayDailyReportUuid << ")...";

        // Create empty task list for initial creation
        QList<QPair<QString, double>> tasks;
        // We need to sync to get the actual uuid after creation
       ApiManager::instance().createDailyReport(
            applicantId,
            applicantName,
            today,
            todayMonth,
            todayWeek,
            tasks
        );
    }
}

void CloudSessionManager::addSession(const WorkSession& session) {
    QString date = session.date;
    if (!buffer.contains(date)) {
        buffer[date] = QList<CloudWorkRecord>();
    }

    CloudWorkRecord record = CloudWorkRecord::fromWorkSession(session);
    buffer[date].append(record);
    qDebug() << "添加会话到buffer:" << date << " activity=" << session.activity << " duration=" << session.durationHours;
}

void CloudSessionManager::updateSession(const WorkSession& session) {
    // Update in all dates
    for (auto& dateRecords : buffer) {
        for (auto& record : dateRecords) {
            if (record.uuid == session.id) {
                record.taskDescription = session.activity;
                record.workingHours = session.durationHours;
                record.isSynced = false;
                qDebug() << "更新会话:" << session.id << " new activity=" << session.activity;
                return;
            }
        }
    }
}

void CloudSessionManager::deleteSession(const QString& sessionId) {
    for (auto& dateRecords : buffer) {
        int beforeSize = dateRecords.size();
        dateRecords.erase(
            std::remove_if(dateRecords.begin(), dateRecords.end(),
                [sessionId](const CloudWorkRecord& r) { return r.uuid == sessionId; }),
            dateRecords.end()
        );
        if (dateRecords.size() < beforeSize) {
            qDebug() << "删除会话:" << sessionId;
            return;
        }
    }
}

void CloudSessionManager::syncToday() {
    QString today = getCurrentDate();
    if (!buffer.contains(today) || buffer[today].isEmpty()) {
        qDebug() << "今天没有会话需要同步";
        return;
    }

    // If daily report UUID not set, create it first
    if (todayDailyReportUuid.isEmpty()) {
        qDebug() << "日报UUID未设置，先创建今日日报...";
        createTodayDailyReportIfNotExist();
        return;
    }

    qDebug() << "正在同步今天" << buffer[today].size() << "条会话到云端...";

    // Create task list for today
    QList<QPair<QString, double>> tasks;
    for (const auto& record : buffer[today]) {
        tasks.append(qMakePair(record.taskDescription, record.workingHours));
    }

    ApiManager::instance().syncDailyReport(
        todayDailyReportUuid,
        applicantId,
        applicantName,
        today,
        todayMonth,
        todayWeek,
        tasks
    );
}

QList<WorkSession> CloudSessionManager::getSessions(const QString& date) const {
    QList<WorkSession> sessions;
    if (!buffer.contains(date)) return sessions;

    for (const auto& record : buffer[date]) {
        WorkSession session;
        session.id = record.uuid;
        session.date = record.date;
        session.startTime = "";  // Not stored in cloud
        // Synced records are completed (have end time), unsynced are active
        session.endTime = record.isSynced ? "synced" : "";
        session.durationHours = record.workingHours;
        session.activity = record.taskDescription;
        session.workType = "开发";  // Default work type
        sessions.append(session);
    }

    return sessions;
}

QList<WorkSession> CloudSessionManager::getTodaySessions() const {
    return getSessions(getCurrentDate());
}

WorkSession CloudSessionManager::getActiveSessionForDate(const QString& date) {
    QList<WorkSession> sessions = getSessions(date);
    for (const WorkSession& s : sessions) {
        if (s.endTime.isEmpty()) return s;
    }
    return WorkSession();
}

DailyStatistics CloudSessionManager::getTodayStatistics() const {
    QList<WorkSession> sessions = getTodaySessions();
    DailyStatistics stat;
    stat.date = getCurrentDate();
    stat.totalHours = 0;
    stat.sessionCount = sessions.size();

    for (const WorkSession& s : sessions) {
        stat.totalHours += s.durationHours;
    }

    return stat;
}

DailyStatistics CloudSessionManager::getStatisticsForDate(const QString& date) const {
    QList<WorkSession> sessions = getSessions(date);
    DailyStatistics stat;
    stat.date = date;
    stat.totalHours = 0;
    stat.sessionCount = sessions.size();

    for (const WorkSession& s : sessions) {
        stat.totalHours += s.durationHours;
    }

    return stat;
}

void CloudSessionManager::parseDailyReportDetails(const QJsonArray& tasks, const QString& date) {
    buffer[date] = QList<CloudWorkRecord>();

    for (const auto& value : tasks) {
        if (!value.isObject()) continue;
        QJsonObject obj = value.toObject();

        CloudWorkRecord record;
        record.uuid = obj["uuid"].toString();
        record.taskDescription = obj["taskDescription"].toString();
        record.workingHours = obj["workingHours"].toDouble();
        record.isSynced = true;
        record.date = date;

        buffer[date].append(record);
        qDebug() << "解析日报详情:" << date << record.taskDescription << " " << record.workingHours << "小时";
    }

    qDebug() << date << "共解析" << buffer[date].size() << "条会话";
}

const QMap<QString, QList<CloudWorkRecord>>& CloudSessionManager::getBuffer() const {
    return buffer;
}

void CloudSessionManager::clearBuffer() {
    buffer.clear();
}

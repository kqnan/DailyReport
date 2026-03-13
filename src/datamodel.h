#pragma once
#include <QString>
#include <QDateTime>
#include <QJsonArray>
#include <optional>
#include <QList>
#include <QPair>

struct WorkSession {
    QString id;
    QString date;           // YYYY-MM-DD
    QString startTime;      // ISO 8601
    QString endTime;        // ISO 8601, optional
    double durationHours;
    QString activity;
    QString workType;

    bool isActive() const { return endTime.isEmpty(); }
    double calculateDuration() const;
};

struct DailyStatistics {
    QString date;
    double totalHours;
    int sessionCount;
};

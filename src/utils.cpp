#include "utils.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>

QString formatDuration(double hours) {
    int h = static_cast<int>(hours);
    int m = static_cast<int>(qRound((hours - h) * 60));

    if (h == 0) return QString::number(m) + "分钟";
    else if (m == 0) return QString::number(h) + "小时";
    else return QString::number(h) + "小时" + QString::number(m) + "分钟";
}

QString getStorageDirectory() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    dir.mkpath("work_sessions");
    return dir.filePath("work_sessions");
}

QString getFilenameForDate(const QString &date) {
    // date: "2026-03-12" -> "sessions_2026_03.json"
    QString yearMonth = date.left(7).replace("-", "_");
    return QString("sessions_%1.json").arg(yearMonth);
}

QString getCurrentDate() {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd");
}

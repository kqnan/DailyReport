#include "datamodel.h"
#include <QDebug>

double WorkSession::calculateDuration() const {
    qDebug() << "[CALC DURATION]";
    qDebug() << "  startTime=" << startTime << "(empty:" << startTime.isEmpty() << ")";
    qDebug() << "  endTime=" << endTime << "(empty:" << endTime.isEmpty() << ")";

    if (endTime.isEmpty()) {
        qDebug() << "  -> endTime is empty, returning 0.0";
        return 0.0;
    }

    if (startTime.isEmpty()) {
        qDebug() << "  -> startTime is empty, returning 0.1 (6 min)";
        return 0.1;
    }

    QDateTime start = QDateTime::fromString(startTime, Qt::ISODate);
    QDateTime end = QDateTime::fromString(endTime, Qt::ISODate);

    qDebug() << "  start valid=" << start.isValid() << ", end valid=" << end.isValid();

    if (!start.isValid() || !end.isValid()) {
        qDebug() << "  -> Invalid datetime, returning 0.1";
        return 0.1;
    }

    double seconds = start.secsTo(end);
    double hours = qRound(seconds / 3600.0 * 10) / 10;

    qDebug() << "  seconds=" << seconds << ", hours=" << hours;

    double result = hours < 0.1 ? 0.1 : hours;
    qDebug() << "  -> returning" << result;
    return result;
}

#include "datamodel.h"

double WorkSession::calculateDuration() const {
    if (endTime.isEmpty()) return 0.0;

    QDateTime start = QDateTime::fromString(startTime, Qt::ISODate);
    QDateTime end = QDateTime::fromString(endTime, Qt::ISODate);

    double seconds = start.secsTo(end);
    double hours = qRound(seconds / 3600.0 * 10) / 10;

    // Minimum duration is 0.1 hours (6 minutes)
    return hours < 0.1 ? 0.1 : hours;
}

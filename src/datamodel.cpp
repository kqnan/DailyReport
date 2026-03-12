#include "datamodel.h"

double WorkSession::calculateDuration() const {
    if (endTime.isEmpty()) return 0.0;

    QDateTime start = QDateTime::fromString(startTime, Qt::ISODate);
    QDateTime end = QDateTime::fromString(endTime, Qt::ISODate);

    double seconds = start.secsTo(end);
    return qRound(seconds / 3600.0 * 10) / 10;
}

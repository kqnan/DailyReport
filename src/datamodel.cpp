#include "datamodel.h"
#include <QDebug>

double WorkSession::calculateDuration() const {
    if (endTime.isEmpty()) return 0.0;
    if (startTime.isEmpty()) return 0.1;

    QDateTime start = QDateTime::fromString(startTime, Qt::ISODate);
    QDateTime end = QDateTime::fromString(endTime, Qt::ISODate);

    if (!start.isValid() || !end.isValid()) {
        return 0.1;
    }

    double seconds = start.secsTo(end);
    // Use 10.0 instead of 10 to ensure floating-point division
    double hours = qRound(seconds / 3600.0 * 10) / 10.0;

    return hours < 0.1 ? 0.1 : hours;
}

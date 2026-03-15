#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QtTest>
#include "../src/datamodel.h"

class TestEmptyStart : public QObject {
    Q_OBJECT
private slots:
    void testEmptyStart() {
        QString emptyStart = "";
        QString endTime = "2026-03-15T10:00:00";

        QDateTime start = QDateTime::fromString(emptyStart, Qt::ISODate);
        QDateTime end = QDateTime::fromString(endTime, Qt::ISODate);

        qDebug() << "Empty start time:";
        qDebug() << "  Start parsed:" << start << "valid:" << start.isValid();
        qDebug() << "  End parsed:" << end << "valid:" << end.isValid();

        if (start.isValid() && end.isValid()) {
            double seconds = start.secsTo(end);
            qDebug() << "  Seconds:" << seconds;
            qDebug() << "  Hours:" << seconds / 3600.0;
        } else {
            qDebug() << "  Cannot calculate - invalid datetime";
        }
    }

    void testCalculateDurationWithEmptyStart() {
        WorkSession session;
        session.startTime = "";  // Empty!
        session.endTime = "2026-03-15T10:00:00";

        double duration = session.calculateDuration();

        // secsTo returns negative when start is invalid, so hours < 0.1, returns 0.1
        qDebug() << "Duration with empty start:" << duration;
        QCOMPARE(duration, 0.1);  // Minimum value
    }

    void testCalculateDurationWithValid1Hour() {
        WorkSession session;
        session.startTime = "2026-03-15T09:00:00";
        session.endTime = "2026-03-15T10:00:00";

        double duration = session.calculateDuration();

        qDebug() << "Duration with 1 hour:" << duration;
        QCOMPARE(duration, 1.0);
    }
};

QTEST_MAIN(TestEmptyStart)
#include "test_empty_start.moc"

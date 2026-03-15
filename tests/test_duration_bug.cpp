/**
 * Test to reproduce user bug report:
 * "Click start shift, come back 1 hour later, click end shift,
 *  duration shows only 6 minutes (0.1 hours)"
 */
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QTest>
#include "../src/datamodel.h"

class TestDurationBug : public QObject {
    Q_OBJECT

private slots:
    void test_1_hour_duration() {
        WorkSession session;
        // Simulate clicking "start shift" at 09:00
        session.startTime = QDateTime::currentDateTime().addSecs(-3600).toString(Qt::ISODate);
        session.endTime = "";
        session.durationHours = 0;

        qDebug() << "Start time:" << session.startTime;

        // Simulate clicking "end shift" now
        session.endTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        session.durationHours = session.calculateDuration();

        qDebug() << "End time:" << session.endTime;
        qDebug() << "Calculated duration:" << session.durationHours << "hours";
        qDebug() << "Expected: ~1.0 hours";

        // Duration should be approximately 1.0 hours (allowing for test execution time)
        QVERIFY(session.durationHours >= 0.9);
        QVERIFY(session.durationHours <= 1.1);
    }

    void test_fixed_times_1_hour() {
        WorkSession session;
        session.startTime = "2026-03-15T09:00:00";
        session.endTime = "2026-03-15T10:00:00";

        double duration = session.calculateDuration();

        qDebug() << "Fixed times duration:" << duration;
        QCOMPARE(duration, 1.0);
    }

    void test_parse_iso_date() {
        QString startTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        qDebug() << "Current time ISO:" << startTime;

        QDateTime parsed = QDateTime::fromString(startTime, Qt::ISODate);
        qDebug() << "Parsed:" << parsed;
        qDebug() << "Is valid:" << parsed.isValid();

        QVERIFY(parsed.isValid());
    }
};

QTEST_MAIN(TestDurationBug)
#include "test_duration_bug.moc"

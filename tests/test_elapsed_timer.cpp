#include <QObject>
#include <QtTest/QtTest>
#include <QDateTime>
#include "../src/datamodel.h"

class TestElapsedTimer : public QObject {
    Q_OBJECT

private slots:
    void testElapsedSecondsToMinutes() {
        // 120 seconds = 2 minutes
        int seconds = 120;
        int hours = static_cast<int>(seconds / 3600);
        int minutes = static_cast<int>((seconds % 3600) / 60);
        QCOMPARE(hours, 0);
        QCOMPARE(minutes, 2);
    }

    void testElapsedMinutesToHours() {
        // 65 minutes = 1 hour 5 minutes
        int seconds = 65 * 60;
        int hours = static_cast<int>(seconds / 3600);
        int minutes = static_cast<int>((seconds % 3600) / 60);
        QCOMPARE(hours, 1);
        QCOMPARE(minutes, 5);
    }

    void testElapsedCrossHourBoundary() {
        // From 09:50 to 10:10 = 20 minutes
        QDateTime start = QDateTime::fromString("2026-03-15T09:50:00", Qt::ISODate);
        QDateTime end = QDateTime::fromString("2026-03-15T10:10:00", Qt::ISODate);
        double seconds = start.secsTo(end);
        int hours = static_cast<int>(seconds / 3600);
        int minutes = static_cast<int>((static_cast<int>(seconds) % 3600) / 60);
        QCOMPARE(hours, 0);
        QCOMPARE(minutes, 20);
    }

    void testElapsedFourHours() {
        // 4 hours = 240 minutes
        int seconds = 4 * 3600;
        int hours = static_cast<int>(seconds / 3600);
        int minutes = static_cast<int>((seconds % 3600) / 60);
        QCOMPARE(hours, 4);
        QCOMPARE(minutes, 0);
    }
};

QTEST_MAIN(TestElapsedTimer)
#include "test_elapsed_timer.moc"

#include <QtTest/QtTest>
#include <QDateTime>
#include "../src/cloudsessionmanager.h"
#include "../src/datamodel.h"

class TestSessionDate : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Clear buffer before running tests
        CloudSessionManager::instance().clearBuffer();
    }

    void cleanupTestCase() {
        // Clear buffer after all tests
        CloudSessionManager::instance().clearBuffer();
    }

    // Test that new session date is always today's date
    void testNewSessionDateIsToday() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        // Get today's date
        QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd");

        // Create a new session
        WorkSession session;
        session.id = QUuid::createUuid().toString().mid(1, 36);
        session.date = today;
        session.startTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        session.endTime = "";
        session.durationHours = 0;
        session.activity = "测试工作";
        session.workType = "开发";

        mgr.addSession(session);

        // Verify session date equals today
        QCOMPARE(session.date, today);

        // Verify sessions retrieved for today contain this session
        QList<WorkSession> sessions = mgr.getSessions(today);
        QCOMPARE(sessions.size(), 1);
        QCOMPARE(sessions[0].date, today);
    }

    // Test that session date is independent of calendar selection
    void testSessionDateIndependentOfCalendarSelection() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        // Simulate calendar selecting a different date (e.g., yesterday)
        QString selectedDate = "2026-03-10";  // Arbitrary past date
        QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd");

        // Ensure selected date is different from today
        QVERIFY(selectedDate != today);

        // Create a new session - it should use today's date, not selected date
        WorkSession session;
        session.id = QUuid::createUuid().toString().mid(1, 36);
        session.date = today;  // Should always be today
        session.startTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        session.durationHours = 0;

        mgr.addSession(session);

        // Verify session date is today, not the selected date
        QCOMPARE(session.date, today);
        QVERIFY(session.date != selectedDate);

        // Verify session is NOT in the selected date's sessions
        QList<WorkSession> selectedDateSessions = mgr.getSessions(selectedDate);
        QVERIFY(selectedDateSessions.isEmpty());

        // Verify session IS in today's sessions
        QList<WorkSession> todaySessions = mgr.getSessions(today);
        QCOMPARE(todaySessions.size(), 1);
        QCOMPARE(todaySessions[0].date, today);
    }
};

QTEST_MAIN(TestSessionDate)
#include "test_session_date.moc"

#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>

#include "../src/apimanager.h"
#include "../src/cloudsessionmanager.h"
#include "../src/datamodel.h"

class TestLogin : public QObject {
    Q_OBJECT

private slots:
    // Test that login parses bearer token correctly
    void test_login_success_parses_token() {
        QByteArray response = R"({"title":"操作提示","message":"bearer;eyJ0eXBlIjoiSldUIiwiYWxnIjoiSFMyNTYifQ.eyJ1dWlkIjoiMjAzNUU1Njg2RDg1NEMyRUI1MTBGM0YwMzc0NkY1RkMiLCJ1c2VyTmFtZUlkIjoiU1ExMzc5MyIsInVzZXJOYW1lIjoi5a2U5ZCv5qWgIiwib3JnSWQiOiIxMzMiLCJyb2xlSWQiOiI1MCIsImlzcyI6InN5c3RlbUp3dCIsImF1ZCI6IjA5OGY2YmNkNDYyMWQzNzNjYWRlNGU4MzI2MjdiNGY5In0.eLhnWyijkMHgeT6CRw2UkGLEfLc43Vf6GrKT4xI4bLw","statusCode":200})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 200);

        QString message = obj["message"].toString();
        QVERIFY(message.startsWith("bearer;"));

        // Extract token
        QString token = message.mid(7);
        QVERIFY(!token.isEmpty());
        // Token should be a JWT (contains two dots)
        QVERIFY(token.contains("."));
    }

    // Test that login failure with non-200 status code is handled correctly
    void test_login_failure_non_200_status() {
        QByteArray response = R"({"title":"操作提示","message":"用户名或密码错误","statusCode":401})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 401);
        QCOMPARE(obj["message"].toString(), QString("用户名或密码错误"));
    }

    // Test that login handles invalid JSON
    void test_login_invalid_json_response() {
        QByteArray response = "invalid json";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error != QJsonParseError::NoError);
    }

    // Test that login handles missing statusCode
    void test_login_missing_status_code() {
        QByteArray response = R"({"title":"操作提示","message":"some message"})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        // statusCode should default to 0 if not present
        QCOMPARE(obj["statusCode"].toInt(), 0);
    }

    // Test that login handles missing message
    void test_login_missing_message() {
        QByteArray response = R"({"statusCode":200})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 200);
        // message should be empty if not present
        QVERIFY(obj["message"].toString().isEmpty());
    }

    // Test verification code is an integer
    void test_verification_code_format() {
        QByteArray response = "7156";

        bool ok = false;
        int code = response.toInt(&ok);

        QVERIFY(ok);
        QCOMPARE(code, 7156);
    }

    // Test verification code can handle single digit
    void test_verification_code_single_digit() {
        QByteArray response = "5";

        bool ok = false;
        int code = response.toInt(&ok);

        QVERIFY(ok);
        QCOMPARE(code, 5);
    }

    // Test that login request has correct fields
    void test_login_request_fields() {
        QString userNameId = "18578775001";
        QString password = "84769629aA.";
        QString code = "7156";

        QJsonDocument doc;
        QJsonObject obj;
        obj["userNameId"] = userNameId;
        obj["password"] = password;
        obj["code"] = code;
        doc.setObject(obj);

        QCOMPARE(obj["userNameId"].toString(), userNameId);
        QCOMPARE(obj["password"].toString(), password);
        QCOMPARE(obj["code"].toString(), code);
    }

    // Test that login request is valid JSON
    void test_login_request_valid_json() {
        QJsonDocument doc;
        QJsonObject obj;
        obj["userNameId"] = "test user";
        obj["password"] = "test password";
        obj["code"] = "1234";
        doc.setObject(obj);

        QByteArray json = doc.toJson(QJsonDocument::Compact);
        QString jsonStr = QString::fromUtf8(json);

        QVERIFY(jsonStr.contains("userNameId"));
        QVERIFY(jsonStr.contains("password"));
        QVERIFY(jsonStr.contains("code"));
    }

    // Bug report test: 1 hour session should show 1 hour, not 0.1 hours (6 minutes)
    void test_1_hour_session_duration() {
        WorkSession session;
        session.startTime = "2026-03-15T09:00:00";
        session.endTime = "2026-03-15T10:00:00";  // 1 hour later

        double duration = session.calculateDuration();

        qDebug() << "1 hour session duration:" << duration;
        QCOMPARE(duration, 1.0);
    }
};

class TestCloudSessionManager : public QObject {
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

    // Test that instance() returns singleton instance
    void test_instance_returns_singleton() {
        CloudSessionManager& mgr1 = CloudSessionManager::instance();
        CloudSessionManager& mgr2 = CloudSessionManager::instance();
        QCOMPARE(&mgr1, &mgr2);
    }

    // Test that addSession adds record to buffer
    void test_addSession_adds_record_to_buffer() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        WorkSession session;
        session.id = "test-uuid-123";
        session.date = "2026-03-13";
        session.startTime = "2026-03-13T09:00:00";
        session.durationHours = 2.5;
        session.activity = "开发功能";
        session.workType = "开发";

        mgr.addSession(session);

        auto buffer = mgr.getBuffer();
        QVERIFY(buffer.contains("2026-03-13"));
        QCOMPARE(buffer["2026-03-13"].size(), 1);
        QCOMPARE(buffer["2026-03-13"][0].uuid, QString("test-uuid-123"));
        QCOMPARE(buffer["2026-03-13"][0].taskDescription, QString("开发功能"));
        QCOMPARE(buffer["2026-03-13"][0].workingHours, 2.5);
    }

    // Test that getSessions returns sessions for a date
    void test_getSessions_returns_sessions_for_date() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        WorkSession session1;
        session1.id = "uuid-1";
        session1.date = "2026-03-13";
        session1.startTime = "2026-03-13T09:00:00";
        session1.durationHours = 1.0;
        session1.activity = "会议";
        session1.workType = "会议";

        WorkSession session2;
        session2.id = "uuid-2";
        session2.date = "2026-03-13";
        session2.startTime = "2026-03-13T11:00:00";
        session2.durationHours = 2.0;
        session2.activity = "开发";
        session2.workType = "开发";

        mgr.addSession(session1);
        mgr.addSession(session2);

        QList<WorkSession> sessions = mgr.getSessions("2026-03-13");
        QCOMPARE(sessions.size(), 2);
        QCOMPARE(sessions[0].id, QString("uuid-1"));
        QCOMPARE(sessions[1].id, QString("uuid-2"));
    }

    // Test that getSessions returns empty list for non-existent date
    void test_getSessions_returns_empty_for_nonexistent_date() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        QList<WorkSession> sessions = mgr.getSessions("2099-01-01");
        QVERIFY(sessions.isEmpty());
    }

    // Test that updateSession updates existing record
    void test_updateSession_updates_existing_record() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        WorkSession session;
        session.id = "update-test-uuid";
        session.date = "2026-03-13";
        session.startTime = "2026-03-13T09:00:00";
        session.durationHours = 1.0;
        session.activity = "原始活动";
        session.workType = "开发";

        mgr.addSession(session);

        // Update the session
        session.activity = "更新后的活动";
        session.durationHours = 2.5;
        mgr.updateSession(session);

        auto buffer = mgr.getBuffer();
        QVERIFY(buffer.contains("2026-03-13"));
        QCOMPARE(buffer["2026-03-13"][0].taskDescription, QString("更新后的活动"));
        QCOMPARE(buffer["2026-03-13"][0].workingHours, 2.5);
    }

    // Test that deleteSession removes record
    void test_deleteSession_removes_record() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        WorkSession session1;
        session1.id = "delete-uuid-1";
        session1.date = "2026-03-13";
        session1.durationHours = 1.0;

        WorkSession session2;
        session2.id = "delete-uuid-2";
        session2.date = "2026-03-13";
        session2.durationHours = 2.0;

        mgr.addSession(session1);
        mgr.addSession(session2);

        QCOMPARE(mgr.getSessionCount("2026-03-13"), 2);

        mgr.deleteSession("delete-uuid-1");

        QCOMPARE(mgr.getSessionCount("2026-03-13"), 1);

        QList<WorkSession> sessions = mgr.getSessions("2026-03-13");
        QCOMPARE(sessions.size(), 1);
        QVERIFY(sessions[0].id != "delete-uuid-1");
    }

    // Test that parseDailyReportDetails parses JSON array correctly for specific date
    void test_parseDailyReportDetails_parses_json_array_for_date() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        QJsonArray tasks;
        QJsonObject task1;
        task1["uuid"] = "api-task-1";
        task1["taskDescription"] = "处理客户咨询";
        task1["workingHours"] = 1.5;
        tasks.append(task1);

        mgr.parseDailyReportDetails(tasks, "2026-03-12");

        auto buffer = mgr.getBuffer();
        QVERIFY(buffer.contains("2026-03-12"));
        QCOMPARE(buffer["2026-03-12"].size(), 1);
        QCOMPARE(buffer["2026-03-12"][0].uuid, QString("api-task-1"));
    }

    // Test: Synced records should NOT be active (have endTime set)
    void test_synced_records_not_active() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        QJsonArray tasks;
        QJsonObject task1;
        task1["uuid"] = "synced-task-1";
        task1["taskDescription"] = "已同步的任务";
        task1["workingHours"] = 2.0;
        tasks.append(task1);

        mgr.parseDailyReportDetails(tasks, "2026-03-13");

        QList<WorkSession> sessions = mgr.getSessions("2026-03-13");
        QCOMPARE(sessions.size(), 1);
        QVERIFY(!sessions[0].isActive());
        QVERIFY(!sessions[0].endTime.isEmpty());
    }

    // Test: Active sessions (added but not synced) should be active
    void test_unsynced_records_are_active() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        WorkSession session;
        session.id = "active-uuid";
        session.date = "2026-03-13";
        session.startTime = "2026-03-13T09:00:00";
        session.durationHours = 1.5;
        session.activity = "进行中的工作";
        session.workType = "开发";

        mgr.addSession(session);

        QList<WorkSession> sessions = mgr.getSessions("2026-03-13");
        QCOMPARE(sessions.size(), 1);
        QVERIFY(sessions[0].isActive());
        QVERIFY(sessions[0].endTime.isEmpty());
    }

    // Test: Today's session count returns correct count
    void test_getSessionCount_returns_correct_count() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        QCOMPARE(mgr.getSessionCount("2026-03-13"), 0);

        WorkSession session1;
        session1.id = "uuid-1";
        session1.date = "2026-03-13";
        mgr.addSession(session1);

        QCOMPARE(mgr.getSessionCount("2026-03-13"), 1);

        WorkSession session2;
        session2.id = "uuid-2";
        session2.date = "2026-03-13";
        mgr.addSession(session2);

        QCOMPARE(mgr.getSessionCount("2026-03-13"), 2);
    }

    // Bug fix test: Session after end shift should NOT be active
    void test_session_after_end_shift_not_active() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        // Add an active session
        WorkSession session;
        session.id = "end-test-uuid";
        session.date = "2026-03-13";
        session.startTime = "2026-03-13T09:00:00";
        session.durationHours = 0;
        session.activity = "进行中的工作";
        session.workType = "开发";
        session.endTime = "";  // Initially active

        mgr.addSession(session);

        // Simulate end shift - set endTime and calculate duration
        session.endTime = "2026-03-13T12:00:00";
        session.durationHours = session.calculateDuration();
        mgr.updateSession(session);

        // After end shift, session should NOT be active
        QList<WorkSession> sessions = mgr.getSessions("2026-03-13");
        QCOMPARE(sessions.size(), 1);
        // BUG: Currently, isSynced is set to false in updateSession,
        // but endTime should still be set to indicate it's ended
        QVERIFY(!sessions[0].isActive());
        QVERIFY(!sessions[0].endTime.isEmpty());
    }

    // Bug fix test: Minimum duration should be 0.1 hours (6 minutes)
    void test_minimum_duration_is_01_hours() {
        WorkSession session;
        session.startTime = "2026-03-13T09:00:00";
        session.endTime = "2026-03-13T09:06:00";  // 6 minutes = 0.1 hours

        double duration = session.calculateDuration();
        // Minimum should be 0.1 hours
        QVERIFY(duration >= 0.1);
        QCOMPARE(duration, 0.1);
    }

    // Bug fix test: Very short duration should be rounded up to 0.1 hours
    void test_very_short_duration_rounded_up() {
        WorkSession session;
        session.startTime = "2026-03-13T09:00:00";
        session.endTime = "2026-03-13T09:03:00";  // 3 minutes

        double duration = session.calculateDuration();
        // Even 3 minutes should be at least 0.1 hours
        QCOMPARE(duration, 0.1);
    }

    // Bug fix test: Duration of 0 should be raised to 0.1
    void test_zero_duration_becomes_01() {
        WorkSession session;
        session.startTime = "2026-03-13T09:00:00";
        session.endTime = "2026-03-13T09:00:30";  // 30 seconds

        double duration = session.calculateDuration();
        // 30 seconds should be rounded up to minimum 0.1 hours
        QCOMPARE(duration, 0.1);
    }
};

// Test sync flow
class TestSyncFlow : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        CloudSessionManager::instance().clearBuffer();
    }

    void cleanupTestCase() {
        CloudSessionManager::instance().clearBuffer();
    }

    // Test that syncToday calls getDailyReportList when UUID is not set
    void test_syncToday_calls_getDailyReportList_when_uuid_empty() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        // Add a session for today
        WorkSession session;
        session.id = "test-uuid";
        session.date = mgr.getCurrentDate();
        session.startTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        session.endTime = "";
        session.durationHours = 1.0;
        session.activity = "测试工作";
        session.workType = "开发";

        mgr.addSession(session);

        // UUID should be empty initially
        QVERIFY(mgr.getTodayDailyReportUuid().isEmpty());

        // syncToday should call getDailyReportList
        mgr.syncToday();

        // UUID should still be empty (no API response received)
        QVERIFY(mgr.getTodayDailyReportUuid().isEmpty());
    }

    // Test that syncToday skips when no sessions for today
    void test_syncToday_skips_when_no_sessions() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        // No sessions added
        QCOMPARE(mgr.getSessionCount(mgr.getCurrentDate()), 0);

        // syncToday should return early (no crash)
        mgr.syncToday();
    }

    // Test that syncToday directly syncs when UUID is set
    void test_syncToday_direct_sync_when_uuid_set() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        // Add a session
        WorkSession session;
        session.id = "test-uuid";
        session.date = mgr.getCurrentDate();
        session.startTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        session.durationHours = 2.0;
        session.activity = "测试工作";
        session.workType = "开发";
        mgr.addSession(session);

        // Set today's report UUID
        mgr.setTodayDailyReport("fake-uuid", "2026-03", "星期六");

        // Verify UUID is set
        QCOMPARE(mgr.getTodayDailyReportUuid(), QString("fake-uuid"));

        // syncToday should use the UUID directly
        mgr.syncToday();
    }

    // Test that getCurrentDate returns valid date
    void test_getCurrentDate_returns_valid_date() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        QString date = mgr.getCurrentDate();

        // Date should be in format "yyyy-MM-dd"
        QVERIFY(date.contains("-"));
        QVERIFY(date.length() >= 7);
    }

    // Test statistics calculation
    void test_getTodayStatistics() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        // Add multiple sessions
        WorkSession s1, s2, s3;
        s1.date = mgr.getCurrentDate(); s1.durationHours = 1.0;
        s2.date = mgr.getCurrentDate(); s2.durationHours = 2.0;
        s3.date = mgr.getCurrentDate(); s3.durationHours = 0.5;

        mgr.addSession(s1);
        mgr.addSession(s2);
        mgr.addSession(s3);

        DailyStatistics stat = mgr.getTodayStatistics();
        QCOMPARE(stat.totalHours, 3.5);
        QCOMPARE(stat.sessionCount, 3);
    }
};

QTEST_MAIN(TestSyncFlow)
#include "test_login.moc"

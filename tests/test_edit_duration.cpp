#include <QtTest/QtTest>
#include <QDateTime>
#include "../src/cloudsessionmanager.h"
#include "../src/datamodel.h"

class TestEditDuration : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        CloudSessionManager::instance().clearBuffer();
    }

    void cleanupTestCase() {
        CloudSessionManager::instance().clearBuffer();
    }

    // 测试：编辑工作时长后，durationHours 和 endTime 正确更新
    void testEditDurationUpdatesEndTime() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        // 创建一个已结束的会话
        WorkSession session;
        session.id = QUuid::createUuid().toString().mid(1, 36);
        session.date = "2026-03-19";
        session.startTime = "2026-03-19T09:00:00";
        session.endTime = "2026-03-19T10:30:00";  // 1.5 hours
        session.durationHours = 1.5;
        session.activity = "测试工作";
        session.workType = "开发";

        mgr.addSession(session);

        // 模拟编辑：将时长从 1.5 改为 2.0 小时
        double newDuration = 2.0;
        QDateTime startDt = QDateTime::fromString(session.startTime, Qt::ISODate);
        QDateTime newEndDt = startDt.addSecs(qRound(newDuration * 3600));

        // 更新会话
        session.durationHours = newDuration;
        session.endTime = newEndDt.toString(Qt::ISODate);
        mgr.updateSession(session);

        // 验证更新后的会话
        QList<WorkSession> sessions = mgr.getSessions("2026-03-19");
        QCOMPARE(sessions.size(), 1);
        QCOMPARE(sessions[0].durationHours, 2.0);

        QDateTime expectedEndDt = QDateTime::fromString("2026-03-19T11:00:00", Qt::ISODate);
        QDateTime actualEndDt = QDateTime::fromString(sessions[0].endTime, Qt::ISODate);
        QCOMPARE(actualEndDt, expectedEndDt);
    }

    // 测试：未结束的会话不应被编辑（模拟检查）
    void testUnfinishedSessionCannotBeEdited() {
        CloudSessionManager& mgr = CloudSessionManager::instance();
        mgr.clearBuffer();

        // 创建一个未结束的会话
        WorkSession session;
        session.id = QUuid::createUuid().toString().mid(1, 36);
        session.date = "2026-03-19";
        session.startTime = "2026-03-19T09:00:00";
        session.endTime = "";  // 未结束
        session.durationHours = 0;
        session.activity = "进行中工作";
        session.workType = "开发";

        mgr.addSession(session);

        // 验证会话确实是未结束状态
        QList<WorkSession> sessions = mgr.getSessions("2026-03-19");
        QCOMPARE(sessions.size(), 1);
        QVERIFY(sessions[0].endTime.isEmpty());

        // 实际编辑阻止逻辑在 MainWindow::onEditSession 中测试
        // 这里仅验证数据模型支持此检查
    }
};

QTEST_MAIN(TestEditDuration)
#include "test_edit_duration.moc"

# 编辑工作时长功能实现计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在编辑对话框中添加工作时长编辑功能，允许用户修改已结束工作记录的时长，并自动重新计算结束时间。

**Architecture:** 修改 `MainWindow::onEditSession()` 函数，添加时长输入验证、新结束时间计算逻辑。未结束的会话直接返回不显示对话框。

**Tech Stack:** Qt 6 (QLineEdit, QDoubleValidator, QDateTime), C++20

---

## 文件结构

**修改文件:**
- `src/mainwindow.cpp`: 修改 `onEditSession()` 函数，添加时长编辑功能和未结束会话检查

**测试文件:**
- `tests/test_edit_duration.cpp`: 新建单元测试文件，测试编辑时长功能

---

## Chunk 1: 实现编辑工作时长功能

### Task 1: 阻止未结束会话打开编辑对话框

**Files:**
- Modify: `src/mainwindow.cpp:354-368` (onEditSession 函数开头)

- [ ] **Step 1: 添加未结束会话检查**

在 `onEditSession()` 函数中，找到 session 后，检查 `endTime` 是否为空，如果为空则直接返回。

```cpp
void MainWindow::onEditSession() {
    QListWidgetItem *item = sessionListWidget->currentItem();
    if (!item) return;

    QString id = item->data(Qt::UserRole).toString();
    WorkSession *session = nullptr;

    for (WorkSession &s : currentSessions) {
        if (s.id == id) {
            session = &s;
            break;
        }
    }

    if (!session) return;

    // 阻止未结束会话编辑
    if (session->endTime.isEmpty()) {
        return;
    }

    // ... 继续原有代码
```

- [ ] **Step 2: 构建并验证**

Run: `cd /root/DailyReport/.worktrees/edit-duration/build && cmake --build .`
Expected: Build succeeds with no errors

- [ ] **Step 3: Commit**

```bash
git add src/mainwindow.cpp
git commit -m "feat: prevent editing unfinished sessions"
```

---

### Task 2: 添加工作时长编辑框

**Files:**
- Modify: `src/mainwindow.cpp:369-400` (在对话框布局中添加时长输入)

- [ ] **Step 1: 添加 QLineEdit 和 QDoubleValidator**

在创建 `activityEdit` 和 `workTypeCombo` 之后，添加时长编辑框：

```cpp
    QPlainTextEdit *activityEdit = new QPlainTextEdit(session->activity);
    QComboBox *workTypeCombo = new QComboBox();
    workTypeCombo->addItems({"开发", "会议", "调试", "文档", "其他"});
    workTypeCombo->setCurrentText(session->workType);

    // 添加工作时长编辑框
    QLineEdit *durationEdit = new QLineEdit();
    durationEdit->setText(QString::number(session->durationHours, 'f', 1));
    QDoubleValidator *validator = new QDoubleValidator(0.1, 24.0, 1, durationEdit);
    validator->setNotation(QDoubleValidator::StandardNotation);
    durationEdit->setValidator(validator);

    layout->addWidget(new QLabel("工作内容:"));
    layout->addWidget(activityEdit);
    layout->addWidget(new QLabel("工作类型:"));
    layout->addWidget(workTypeCombo);
    layout->addWidget(new QLabel("工作时长(小时):"));  // 添加标签
    layout->addWidget(durationEdit);                    // 添加输入框
```

- [ ] **Step 2: 构建并验证**

Run: `cd /root/DailyReport/.worktrees/edit-duration/build && cmake --build .`
Expected: Build succeeds with no errors

- [ ] **Step 3: Commit**

```bash
git add src/mainwindow.cpp
git commit -m "feat: add duration edit field in edit dialog"
```

---

### Task 3: 实现保存时更新时长和结束时间

**Files:**
- Modify: `src/mainwindow.cpp:405-414` (保存逻辑)

- [ ] **Step 1: 修改保存逻辑**

将保存时的逻辑替换为：

```cpp
    if (dialog->exec() == QDialog::Accepted) {
        double newDuration = durationEdit->text().toDouble();

        // 计算新的结束时间: startTime + durationHours
        QDateTime startDt = QDateTime::fromString(session->startTime, Qt::ISODate);
        int secondsToAdd = qRound(newDuration * 3600);  // 小时转秒
        QDateTime newEndDt = startDt.addSecs(secondsToAdd);

        session->durationHours = newDuration;
        session->endTime = newEndDt.toString(Qt::ISODate);
        session->activity = activityEdit->toPlainText();
        session->workType = workTypeCombo->currentText();

        CloudSessionManager::instance().updateSession(*session);
        loadSessions(session->date);
    }

    delete dialog;
```

- [ ] **Step 2: 构建并验证**

Run: `cd /root/DailyReport/.worktrees/edit-duration/build && cmake --build .`
Expected: Build succeeds with no errors

- [ ] **Step 3: Commit**

```bash
git add src/mainwindow.cpp
git commit -m "feat: calculate new end time when editing duration"
```

---

### Task 4: 添加单元测试

**Files:**
- Create: `tests/test_edit_duration.cpp`
- Modify: `CMakeLists.txt` (添加测试可执行文件)

- [ ] **Step 1: 创建单元测试文件**

```cpp
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
```

- [ ] **Step 2: 修改 CMakeLists.txt 添加测试**

在 `CMakeLists.txt` 中找到其他测试的添加位置，添加新的测试：

```cmake
add_executable(test_edit_duration
    tests/test_edit_duration.cpp
    src/datamodel.h
    src/datamodel.cpp
    src/sessionmanager.h
    src/sessionmanager.cpp
    src/cloudsessionmanager.h
    src/cloudsessionmanager.cpp
    src/apimanager.h
    src/apimanager.cpp
    src/utils.h
    src/utils.cpp
)

target_link_libraries(test_edit_duration PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Network
    Qt6::Test
)
```

- [ ] **Step 3: 构建测试**

Run: `cd /root/DailyReport/.worktrees/edit-duration/build && cmake .. && cmake --build .`
Expected: Build succeeds, test_edit_duration executable created

- [ ] **Step 4: Commit**

```bash
git add tests/test_edit_duration.cpp CMakeLists.txt
git commit -m "test: add unit tests for edit duration feature"
```

---

## 验证清单

- [ ] 双击未结束记录时，不弹出编辑对话框
- [ ] 双击已结束记录时，弹出编辑对话框并显示当前工作时长（1位小数）
- [ ] 工作时长输入框只能输入正数，范围 0.1-24.0，最多1位小数
- [ ] 保存后，buffer 中的 workingHours 和 endTime 正确更新
- [ ] 新的 endTime = startTime + (durationHours * 3600 seconds)
- [ ] 单元测试编译通过

---

## 提交历史

预期提交顺序:
1. `feat: prevent editing unfinished sessions`
2. `feat: add duration edit field in edit dialog`
3. `feat: calculate new end time when editing duration`
4. `test: add unit tests for edit duration feature`

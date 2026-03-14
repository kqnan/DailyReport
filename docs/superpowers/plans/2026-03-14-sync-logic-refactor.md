# 同步逻辑重构 Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 重构同步逻辑，使今日日报的创建/获取仅在用户点击"同步"按钮时进行，而不是登录时自动创建。

**Architecture:**
- 保持 `CloudSessionManager::syncToday()` 作为核心同步入口
- 移除 `MainWindow::onDailyReportListReceived()` 中的自动创建逻辑
- 简化回调函数，创建失败直接终止同步
- 登录时仅加载历史日报数据，不创建今日日报

**Tech Stack:** Qt 6.6, C++20, QtTest

---

## Chunk 1: CloudSessionManager 头文件修改

### Task 1: 修改 cloudsessionmanager.h

**Files:**
- Modify: `src/cloudsessionmanager.h`

- [ ] **Step 1: 添加新方法声明**

在 public 部分添加：
```cpp
    // Create today's daily report (unconditional, caller decides)
    void createTodayDailyReport();

    // Check and sync today's daily report after getting list
    void checkAndSyncTodayDailyReport(const QJsonArray& reports);
```

在现有 `getTodayDailyReportUuid()` 后添加 doses:

- [ ] **Step 2: 移除私有方法声明**

在 private 部分，将 `createTodayDailyReportIfNotExist()` 声明改为私有（因为将重命名为 public）：
```cpp
private:
    // ... existing private members ...
    // Move createTodayDailyReportIfNotExist to public or keep as private helper
```

- [ ] **Step 3: 验证头文件语法**

运行：`g++ -std=c++20 -fsyntax-only src/cloudsessionmanager.h`
Expected: No syntax errors

---

## Chunk 2: CloudSessionManager 实现文件修改

### Task 2: 修改 cloudsessionmanager.cpp - loadTodaySessions

**Files:**
- Modify: `src/cloudsessionmanager.cpp:55-67`

- [ ] **Step 1: 移除 loadTodaySessions 中的自动创建逻辑**

修改前：
```cpp
void CloudSessionManager::loadTodaySessions() {
    QString today = getCurrentDate();
    if (todayDailyReportUuid.isEmpty()) {
        qDebug() << "今日日报UUID未设置，尝试创建日报...";
        createTodayDailyReportIfNotExist();
        return;
    }

    qDebug() << "正在获取今日日报详情...";
    ApiManager::instance().getDailyReportDetails(today, applicantId);
}
```

修改后：
```cpp
void CloudSessionManager::loadTodaySessions() {
    QString today = getCurrentDate();
    if (todayDailyReportUuid.isEmpty()) {
        qDebug() << "今日日报UUID未设置，无法加载详情";
        return;
    }

    qDebug() << "正在获取今日日报详情...";
    ApiManager::instance().getDailyReportDetails(today, applicantId);
}
```

- [ ] **Step 2: 运行编译检查**

运行：`cmake --build build --target DailyReport`
Expected: No compilation errors

### Task 3: 修改 cloudsessionmanager.cpp - createTodayDailyReport

**Files:**
- Modify: `src/cloudsessionmanager.cpp:88-111`

- [ ] **Step 1: 将 createTodayDailyReportIfNotExist 改为 createTodayDailyReport**

修改前：
```cpp
void CloudSessionManager::createTodayDailyReportIfNotExist() {
    QString today = getCurrentDate();
    if (todayDailyReportUuid.isEmpty()) {
        qDebug() << "创建今日日报...";
        // ...
    }
}
```

修改后：
```cpp
void CloudSessionManager::createTodayDailyReport() {
    QString today = getCurrentDate();

    qDebug() << "创建今日日报...";
    qDebug() << "申请人:" << applicantId << ":" << applicantName;

    QString month = today.left(7);
    QString week = getCurrentDayOfWeek(today);
    qDebug() << "日期:" << today << " 月份:" << month << " 周:" << week;

    QList<QPair<QString, double>> tasks;
    ApiManager::instance().createDailyReport(
        applicantId, applicantName, today, month, week, tasks
    );
}
```

### Task 4: 添加 checkAndSyncTodayDailyReport 方法

**Files:**
- Modify: `src/cloudsessionmanager.cpp` (在 createTodayDailyReport 后添加)

- [ ] **Step 1: 添加新方法实现**

```cpp
void CloudSessionManager::checkAndSyncTodayDailyReport(const QJsonArray& reports) {
    QString today = getCurrentDate();

    // Check if today's report already has UUID set
    if (!todayDailyReportUuid.isEmpty()) {
        qDebug() << "今日日报UUID已设置，跳过检查";
        return;
    }

    // Find today's report in the list
    for (const QJsonValue& value : reports) {
        QJsonObject report = value.toObject();
        if (report["dailyReportDate"].toString() == today) {
            setTodayDailyReport(
                report["uuid"].toString(),
                report["month"].toString(),
                report["week"].toString()
            );
            qDebug() << "找到今日日报并已设置 UUID";

            // Sync today's sessions
            syncToday();
            return;
        }
    }

    qDebug() << "今日日报不存在，等待用户点击同步后创建";
}
```

- [ ] **Step 2: 运行编译检查**

运行：`cmake --build build --target DailyReport`
Expected: No compilation errors

---

## Chunk 3: MainWindow 头文件修改

### Task 5: 移除MainWindow.h 中的hasTriedCreateTodayReport

**Files:**
- Modify: `src/mainwindow.h:70-71`

- [ ] **Step 1: 删除 hasTriedCreateTodayReport 成员变量**

删除以下行：
```cpp
    // Track if we've already tried to create today's report (to prevent loops)
    bool hasTriedCreateTodayReport = false;
```

- [ ] **Step 2: 验证语法**

运行：`g++ -std=c++20 -fsyntax-only src/mainwindow.h`
Expected: No syntax errors

---

## Chunk 4: MainWindow 实现文件修改

### Task 6: 修改 onDailyReportListReceived

**Files:**
- Modify: `src/mainwindow.cpp:508-570`

- [ ] **Step 1: 简化 onDailyReportListReceived**

修改前（约63行）：
```cpp
void MainWindow::onDailyReportListReceived(const QJsonArray& reports) {
    qDebug() << "获取日报列表成功，共" << reports.size() << "条记录";

    QJsonDocument doc;
    doc.setArray(reports);
    qDebug() << "日报列表原始响应:" << doc.toJson(QJsonDocument::Indented);

    QString today = getCurrentDate();
    CloudSessionManager& mgr = CloudSessionManager::instance();
    bool hasLocalSessions = mgr.getSessionCount(today) > 0;

    if (reports.isEmpty()) {
        if (hasLocalSessions) {
            qDebug() << "日报列表为空但有本地会话，直接尝试创建日报...";
            mgr.createTodayDailyReportIfNotExist();
        } else {
            qDebug() << "日报列表为空，暂无日报记录";
            mgr.loadRecentDaysSessions(3);
        }
        return;
    }

    bool foundTodayReport = false;
    for (const QJsonValue& value : reports) {
        QJsonObject report = value.toObject();
        QString date = report["dailyReportDate"].toString();
        qDebug() << "日报:" << date << "创建人:" << report["creator"].toString();

        if (date == today) {
            mgr.setTodayDailyReport(
                report["uuid"].toString(),
                report["month"].toString(),
                report["week"].toString()
            );
            qDebug() << "找到今日日报:" << today;
            foundTodayReport = true;
        }
    }

    if (!foundTodayReport) {
        qDebug() << "今日日报不存在，尝试创建...";
        mgr.createTodayDailyReportIfNotExist();
        return;
    }

    mgr.loadRecentDaysSessions(3);
    mgr.loadTodaySessions();
    mgr.syncToday();
}
```

修改后（约40行）：
```cpp
void MainWindow::onDailyReportListReceived(const QJsonArray& reports) {
    qDebug() << "获取日报列表成功，共" << reports.size() << "条记录";

    QJsonDocument doc;
    doc.setArray(reports);
    qDebug() << "日报列表原始响应:" << doc.toJson(QJsonDocument::Indented);

    QString today = getCurrentDate();
    CloudSessionManager& mgr = CloudSessionManager::instance();

    // Parse all daily report details for loading history
    for (const QJsonValue& value : reports) {
        QJsonObject report = value.toObject();
        QString date = report["dailyReportDate"].toString();
        mgr.loadDailyReportDetails(date);
    }

    // Check if today's report is already handled by syncToday()
    if (mgr.getTodayDailyReportUuid().isEmpty()) {
        for (const QJsonValue& value : reports) {
            QJsonObject report = value.toObject();
            if (report["dailyReportDate"].toString() == today) {
                mgr.setTodayDailyReport(
                    report["uuid"].toString(),
                    report["month"].toString(),
                    report["week"].toString()
                );
                qDebug() << "找到今日日报，已设置 UUID";
                break;
            }
        }
    }

    // Load recent days' sessions
    mgr.loadRecentDaysSessions(3);
}
```

- [ ] **Step 2: 运行编译检查**

运行：`cmake --build build --target DailyReport`
Expected: No compilation errors

### Task 7: 修改 onDailyReportCreated

**Files:**
- Modify: `src/mainwindow.cpp:572-612`

- [ ] **Step 1: 简化 onDailyReportCreated**

修改前（41行）：
```cpp
void MainWindow::onDailyReportCreated(const QString& message, const QString& status) {
    qDebug() << "日报创建成功:" << message << " " << status;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        QString uuid = obj["uuid"].toString();
        if (!uuid.isEmpty()) {
            qDebug() << "从响应中提取UUID:" << uuid;
            CloudSessionManager::instance().setTodayDailyReport(uuid,
                CloudSessionManager::instance().getTodayMonth(),
                CloudSessionManager::instance().getTodayWeek());

            CloudSessionManager::instance().syncToday();
            return;
        }
    }

    if (message.startsWith("create;")) {
        QString uuid = message.mid(7);
        qDebug() << "从create;前缀提取UUID:" << uuid;
        CloudSessionManager::instance().setTodayDailyReport(uuid,
            CloudSessionManager::instance().getTodayMonth(),
            CloudSessionManager::instance().getTodayWeek());

        CloudSessionManager::instance().syncToday();
        return;
    }

    qDebug() << "创建成功但未返回UUID，获取日报列表以提取新创建的日报...";
    apiManager->getDailyReportList(1, 50);
}
```

修改后（23行）：
```cpp
void MainWindow::onDailyReportCreated(const QString& message, const QString& status) {
    if (status != "创建成功") {
        qDebug() << "日报创建失败:" << message;
        QMessageBox::warning(this, "同步失败", message);
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        QString uuid = obj["uuid"].toString();
        if (!uuid.isEmpty()) {
            CloudSessionManager::instance().setTodayDailyReport(
                uuid,
                CloudSessionManager::instance().getTodayMonth(),
                CloudSessionManager::instance().getTodayWeek()
            );
            CloudSessionManager::instance().syncToday();
            return;
        }
    }

    qDebug() << "创建成功但未返回UUID，无法继续同步";
    QMessageBox::warning(this, "同步失败", "创建日报成功但未返回UUID，请重试");
}
```

- [ ] **Step 2: 运行编译检查**

运行：`cmake --build build --target DailyReport`
Expected: No compilation errors

### Task 8: 修改 onDailyReportCreateFailed

**Files:**
- Modify: `src/mainwindow.cpp:614-644`

- [ ] **Step 1: 简化 onDailyReportCreateFailed**

修改前（30行）：
```cpp
void MainWindow::onDailyReportCreateFailed(const QString& error) {
    qDebug() << "日报创建失败:" << error;

    if (error.contains("已存在") || error.contains("重复")) {
        qDebug() << "日报已存在，不重试创建，直接同步本地会话...";

        static bool hasRetried = false;
        if (!hasRetried) {
            hasRetried = true;
            qDebug() << "获取日报列表以找到UUID...";
            apiManager->getDailyReportList(1, 50);
        } else {
            qDebug() << "已重试过，放弃获取列表，仅清空本地缓存避免循环";
            CloudSessionManager::instance().clearBuffer();
            hasRetried = false;
        }
    } else {
        QMessageBox::warning(this, "同步失败", error);
    }
}
```

修改后（6行）：
```cpp
void MainWindow::onDailyReportCreateFailed(const QString& error) {
    qDebug() << "日报创建失败:" << error;
    QMessageBox::warning(this, "同步失败", error);
}
```

- [ ] **Step 2: 运行编译检查**

运行：`cmake --build build --target DailyReport`
Expected: No compilation errors

---

## Chunk 5: 构建与测试

### Task 9: 构建项目

**Files:**
- Build: `build/`

- [ ] **Step 1: 清理并重新构建**

```bash
cd /root/DailyReport
rm -rf build
mkdir -p build
cd build
cmake ..
cmake --build .
```

Expected: Build succeeds with no errors

- [ ] **Step 2: 运行单元测试**

```bash
./build/test_apimanager -platform offscreen
```

Expected: All 42 tests pass

```bash
./build/test_login -platform offscreen
```

Expected: All existing tests pass

### Task 10: 手动测试

- [ ] **Step 1: 测试同步流程 - 今日日报不存在**

准备：
1. 清空 buffer: `CloudSessionManager::instance().clearBuffer()`
2. 添加今日会话

操作：点击"同步"按钮

期望：
1. 控制台输出"日报UUID未设置，检查今日日报是否存在..."
2. 控制台输出"今日日报不存在，尝试创建..."
3. 创建成功后输出"找到今日日报并已设置 UUID"
4. 同步成功

- [ ] **Step 2: 测试同步流程 - 今日日报已存在**

准备：
1. 手动创建今日日报（通过浏览器或 API）
2. 清空 buffer
3. 添加今日会话

操作：点击"同步"按钮

期望：
1. 找到今日日报并设置 UUID
2. 直接同步成功

- [ ] **Step 3: 测试登录后加载历史数据**

操作：
1. 登录成功

期望：
1. 获取日报列表
2. 解析每个日报的详情
3. buffer 中应包含最近几天的数据
4. **不**创建今日日报

- [ ] **Step 4: 测试同步失败处理**

准备：断开网络连接

操作：点击"同步"按钮

期望：
1. 提示错误信息
2. 不造成死循环
3. 不重复尝试

### Task 11: 提交代码

- [ ] **Step 1: 检查 git 状态**

```bash
git status
```

Expected showing modified files:
- src/cloudsessionmanager.h
- src/cloudsessionmanager.cpp
- src/mainwindow.h
- src/mainwindow.cpp

- [ ] **Step 2: 添加并提交**

```bash
git add src/cloudsessionmanager.h src/cloudsessionmanager.cpp src/mainwindow.h src/mainwindow.cpp
git commit -m "$(cat <<'EOF'
refactor: 重构同步逻辑，仅在点击同步时创建今日日报

- 移除 onDailyReportListReceived 中的自动创建逻辑
- 移除 hasTriedCreateTodayReport 标记
- 简化错误处理，创建失败直接终止同步
- 登录时仅加载历史日报数据

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
EOF
)"
```

- [ ] **Step 3: 推送到远程**

```bash
git push origin master
```

Expected: Push succeeds

---

## Plan Review

After completing each chunk, run the plan reviewer to verify correctness.

**Review questions to ask:**
1. Did you follow the exact code changes specified?
2. Did you run the compilation check after each chunk?
3. Are all test scenarios covered by manual testing?
4. Is the commit message clear and complete?

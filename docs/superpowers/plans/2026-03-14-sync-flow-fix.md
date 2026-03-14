# 同步流程修复 Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 修复登录后自动创建/同步今日日报的问题，确保所有操作只在用户点击"同步"按钮时进行。

**Architecture:** 添加 `m_isSyncRequest` 状态标志区分 `getDailyReportList()` 的调用来源，登录后只记录 UUID 不同步，同步时才执行创建/同步操作。

**Tech Stack:** Qt 6.6, C++20, QtTest

---

## Chunk 1: 头文件修改 - 添加状态标志

### Task 1: 修改 mainwindow.h

**Files:**
- Modify: `src/mainwindow.h`

- [ ] **Step 1: 添加 m_isSyncRequest 成员变量**

在 `private:` 部分添加：
```cpp
    // Sync request flag
    bool m_isSyncRequest = false;
```

完整修改后 `private:` 部分：
```cpp
private:
    // Sync request flag
    bool m_isSyncRequest = false;

    // Buttons
    QPushButton *startButton;
    // ... rest of members
```

- [ ] **Step 2: 验证语法**

Run: `g++ -std=c++20 -fsyntax-only src/mainwindow.h`
Expected: No syntax errors

- [ ] **Step 3: 编译检查**

Run: `cd build && cmake --build . --target DailyReport`
Expected: Build succeeds

- [ ] **Step 4: 提交**

```bash
git add src/mainwindow.h
git commit -m "feat: add m_isSyncRequest flag for sync flow control"
```

---

## Chunk 2: 修改 onSync() 方法

### Task 2: 修改 mainwindow.cpp::onSync()

**Files:**
- Modify: `src/mainwindow.cpp:494-505`

- [ ] **Step 1: 修改 onSync() 方法**

当前代码：
```cpp
void MainWindow::onSync() {
    QString today = getCurrentDate();
    CloudSessionManager& mgr = CloudSessionManager::instance();

    if (!mgr.getBuffer().contains(today) || mgr.getBuffer()[today].isEmpty()) {
        QMessageBox::information(this, "同步", "今天没有记录需要同步");
        return;
    }

    statusLabel->setText("状态：同步中...");
    mgr.syncToday();
}
```

修改为：
```cpp
void MainWindow::onSync() {
    QString today = getCurrentDate();
    CloudSessionManager& mgr = CloudSessionManager::instance();

    if (!mgr.getBuffer().contains(today) || mgr.getBuffer()[today].isEmpty()) {
        QMessageBox::information(this, "同步", "今天没有记录需要同步");
        return;
    }

    statusLabel->setText("状态：同步中...");
    m_isSyncRequest = true;  // 标记为同步请求

    // 检查 UUID 是否存在
    if (!mgr.getTodayDailyReportUuid().isEmpty()) {
        qDebug() << "今日日报 UUID 已存在，直接同步";
        mgr.syncDailyReport(mgr.getTodayDailyReportUuid(), today);
        m_isSyncRequest = false;  // 立即重置
    } else {
        qDebug() << "今日日报 UUID 不存在，获取列表查找";
        apiManager->getDailyReportList(1, 50);
        // m_isSyncRequest 保持 true，等待回调处理
    }
}
```

- [ ] **Step 2: 编译检查**

Run: `cd build && cmake --build . --target DailyReport`
Expected: Build succeeds

- [ ] **Step 3: 提交**

```bash
git add src/mainwindow.cpp
git commit -m "feat: update onSync() to use m_isSyncRequest flag"
```

---

## Chunk 3: 修改 onDailyReportListReceived() 方法

### Task 3: 修改 mainwindow.cpp::onDailyReportListReceived()

**Files:**
- Modify: `src/mainwindow.cpp:507-556`

- [ ] **Step 1: 修改 onDailyReportListReceived() 方法**

当前代码（约 50 行）替换为：
```cpp
void MainWindow::onDailyReportListReceived(const QJsonArray& reports) {
    qDebug() << "获取日报列表成功，共" << reports.size() << "条记录";

    QString today = getCurrentDate();
    CloudSessionManager& mgr = CloudSessionManager::instance();

    // 1. 遍历加载所有日报详情（历史数据）
    for (const QJsonValue& value : reports) {
        QJsonObject report = value.toObject();
        QString date = report["dailyReportDate"].toString();
        mgr.loadDailyReportDetails(date);
    }

    // 2. 查找今日日报
    bool foundToday = false;
    for (const QJsonValue& value : reports) {
        QJsonObject report = value.toObject();
        if (report["dailyReportDate"].toString() == today) {
            mgr.setTodayDailyReport(
                report["uuid"].toString(),
                report["month"].toString(),
                report["week"].toString()
            );
            qDebug() << "找到今日日报，已记录 UUID";
            foundToday = true;
            break;
        }
    }

    // 3. 根据请求来源决定是否执行创建/同步
    if (m_isSyncRequest) {
        if (!foundToday) {
            qDebug() << "今日日报不存在，开始创建...";
            mgr.createTodayDailyReport();
            return;  // 等待创建成功后再同步
        }
        qDebug() << "找到今日日报，开始同步...";
        mgr.syncToday();
        m_isSyncRequest = false;  // 重置标志
    } else {
        // 登录请求：只记录 UUID 和加载数据，不操作
        if (foundToday) {
            qDebug() << "登录后找到今日日报，仅记录 UUID（等待用户点击同步）";
        } else {
            qDebug() << "登录后未找到今日日报（等待用户点击同步后创建）";
        }
    }
}
```

- [ ] **Step 2: 编译检查**

Run: `cd build && cmake --build . --target DailyReport`
Expected: Build succeeds

- [ ] **Step 3: 运行单元测试**

Run: `./build/test_apimanager -platform offscreen`
Expected: All 42 tests pass

Run: `./build/test_login -platform offscreen`
Expected: All 7 tests pass

- [ ] **Step 4: 提交**

```bash
git add src/mainwindow.cpp
git commit -m "fix: prevent auto create/sync on login, only on sync button click"
```

---

## Chunk 4: 错误处理 - 重置状态标志

### Task 4: 修改错误处理回调

**Files:**
- Modify: `src/mainwindow.cpp:558-587`

- [ ] **Step 1: 修改 onDailyReportCreateFailed()**

确保失败时重置 `m_isSyncRequest`：
```cpp
void MainWindow::onDailyReportCreateFailed(const QString& error) {
    qDebug() << "日报创建失败:" << error;
    m_isSyncRequest = false;  // 重置标志
    QMessageBox::warning(this, "同步失败", error);
}
```

- [ ] **Step 2: 确认 onDailyReportCreated() 不需要修改**

当前代码已正确：
```cpp
void MainWindow::onDailyReportCreated(const QString& message, const QString& status) {
    if (status != "创建成功") {
        qDebug() << "日报创建失败:" << message;
        QMessageBox::warning(this, "同步失败", message);
        return;
    }

    qDebug() << "日报创建成功，获取日报列表以提取 UUID...";
    apiManager->getDailyReportList(1, 50);
    // m_isSyncRequest 保持 true，等待 onDailyReportListReceived 处理
}
```

- [ ] **Step 3: 编译检查**

Run: `cd build && cmake --build . --target DailyReport`
Expected: Build succeeds

- [ ] **Step 4: 提交**

```bash
git add src/mainwindow.cpp
git commit -m "fix: reset m_isSyncRequest on create failed"
```

---

## Chunk 5: 验证与测试

### Task 5: 手动测试

**Prerequisites:** 编译成功，所有单元测试通过

- [ ] **Step 1: 测试场景 1 - 登录后云端无今日日报**

操作：
1. 确保云端没有今日日报
2. 登录
3. 观察控制台输出

Expected:
- 控制台输出"登录后未找到今日日报（等待用户点击同步后创建）"
- 不自动创建日报
- UI 正常显示

- [ ] **Step 2: 测试场景 2 - 登录后云端有今日日报**

操作：
1. 确保云端已存在今日日报
2. 登录
3. 观察控制台输出

Expected:
- 控制台输出"登录后找到今日日报，仅记录 UUID（等待用户点击同步）"
- UI 展示云端数据
- 本地会话不同步

- [ ] **Step 3: 测试场景 3 - 点击同步 - 云端无今日日报**

操作：
1. 本地有会话，云端无今日日报
2. 点击同步按钮
3. 观察控制台输出

Expected:
- "今日日报 UUID 不存在，获取列表查找"
- "今日日报不存在，开始创建..."
- 创建成功后"获取日报列表以提取 UUID..."
- 最终"找到今日日报，开始同步..."
- 同步成功

- [ ] **Step 4: 测试场景 4 - 点击同步 - 云端有今日日报**

操作：
1. 本地有会话，云端有今日日报
2. 点击同步按钮
3. 观察控制台输出

Expected:
- UUID 已存在 → "今日日报 UUID 已存在，直接同步"
- 同步成功

- [ ] **Step 5: 提交测试验证**

```bash
git commit --allow-empty -m "test: manual testing passed for sync flow fix"
```

---

## Plan Review

完成所有 Chunk 后，调用 plan-document-reviewer 子代理验证计划执行情况。

**Review context:**
- Spec document: `docs/superpowers/specs/2026-03-14-sync-flow-fix-design.md`
- Plan document: `docs/superpowers/plans/2026-03-14-sync-flow-fix.md`
- Modified files: `src/mainwindow.h`, `src/mainwindow.cpp`

**Review questions:**
1. 代码修改是否符合设计文档？
2. 四种情况是否都能正确处理？
3. 状态标志 `m_isSyncRequest` 是否正确重置？
4. 单元测试是否全部通过？

---

## 验收标准

- [ ] 登录后不自动创建今日日报
- [ ] 登录后不自动同步本地会话
- [ ] 登录后能正确展示云端今日日报数据（如果存在）
- [ ] 点击同步能正确处理四种情况
- [ ] 所有单元测试通过
- [ ] 手动测试通过

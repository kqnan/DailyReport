# 修复同步逻辑实施计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix sync logic so it works correctly when daily report already exists for the current date

**Architecture:**
- Add `dailyReportCreateFailed` signal to ApiManager for error handling
- Add `getCurrentDayOfWeek()` helper function to CloudSessionManager
- Modify `createTodayDailyReportIfNotExist()` to use local month/week calculation
- Add retry logic in `onDailyReportCreated()` to fetch existing daily report if creation fails

**Tech Stack:** Qt 6.6.2, C++20

---

## Chunk 1: Add getCurrentDayOfWeek helper

### Task 1.1: Add getCurrentDayOfWeek declaration

**Files:**
- Modify: `src/cloudsessionmanager.h:104`

- [ ] **Step 1: Add method declaration**

Add after `getCurrentDate()` declaration:

```cpp
QString getCurrentDayOfWeek(const QString& date) const;
```

- [ ] **Step 2: Commit**

```bash
git add src/cloudsessionmanager.h
git commit -m "refactor: add getCurrentDayOfWeek declaration"
```

---

### Task 1.2: Implement getCurrentDayOfWeek

**Files:**
- Modify: `src/cloudsessionmanager.cpp`

- [ ] **Step 1: Add implementation**

Add implementation after `getCurrentDate()`:

```cpp
QString CloudSessionManager::getCurrentDayOfWeek(const QString& date) const {
    QDate qDate = QDate::fromString(date, "yyyy-MM-dd");
    if (!qDate.isValid()) {
        return "星期一";  // Default fallback
    }

    int dayOfWeek = qDate.dayOfWeek();  // 1=Monday, 7=Sunday
    QString days[] = {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"};
    return days[dayOfWeek - 1];
}
```

- [ ] **Step 2: Build and verify compilation**

```bash
cd /root/DailyReport/build && make -j$(nproc)
```

Expected: No errors

- [ ] **Step 3: Commit**

```bash
git add src/cloudsessionmanager.cpp
git commit -m "feat: implement getCurrentDayOfWeek function"
```

---

## Chunk 2: Modify createTodayDailyReportIfNotExist

### Task 2.1: Fix month/week parameters

**Files:**
- Modify: `src/cloudsessionmanager.cpp:78-96`

- [ ] **Step 1: Update createTodayDailyReportIfNotExist**

Replace the function with:

```cpp
void CloudSessionManager::createTodayDailyReportIfNotExist() {
    QString today = getCurrentDate();
    if (todayDailyReportUuid.isEmpty()) {
        qDebug() << "创建今日日报...";
        qDebug() << "申请人:" << applicantId << ":" << applicantName;

        // Use local calculation for month and week
        QString month = today.left(7);  // "2026-03"
        QString week = getCurrentDayOfWeek(today);  // "星期三"

        qDebug() << "日期:" << today << " 月份:" << month << " 周:" << week;

        // Create empty task list for initial creation
        QList<QPair<QString, double>> tasks;
        ApiManager::instance().createDailyReport(
            applicantId,
            applicantName,
            today,
            month,
            week,
            tasks
        );
    }
}
```

- [ ] **Step 2: Build and verify**

```bash
cd /root/DailyReport/build && make -j$(nproc)
```

Expected: No errors

- [ ] **Step 3: Commit**

```bash
git add src/cloudsessionmanager.cpp
git commit -m "fix: use local month/week calculation for daily report"
```

---

## Chunk 3: Add dailyReportCreateFailed signal

### Task 3.1: Add signal declaration

**Files:**
- Modify: `src/apimanager.h`

- [ ] **Step 1: Add signal declaration**

Add after `dailyReportCreated` signal:

```cpp
void dailyReportCreateFailed(const QString& error);
```

- [ ] **Step 2: Commit**

```bash
git add src/apimanager.h
git commit -m "feat: add dailyReportCreateFailed signal"
```

---

### Task 3.2: Update createDailyReport response handling

**Files:**
- Modify: `src/apimanager.cpp:218-232`

- [ ] **Step 1: Update response handling**

Replace the response handling section:

```cpp
QJsonObject obj = doc.object();
int statusCode = obj["statusCode"].toInt();

if (statusCode == 200) {
    QString message = obj["message"].toString();
    qDebug() << "创建日报API成功:" << message;
    emit dailyReportCreated(message, "创建成功");
} else {
    QString errorMsg = obj["message"].toString();
    qDebug() << "创建日报API失败:" << errorMsg;
    emit dailyReportCreateFailed(errorMsg);
}
```

- [ ] **Step 2: Build and verify**

```bash
cd /root/DailyReport/build && make -j$(nproc)
```

Expected: No errors

- [ ] **Step 3: Commit**

```bash
git add src/apimanager.cpp
git commit -m "feat: improve createDailyReport error handling"
```

---

## Chunk 4: Modify MainWindow to handle creation failure

### Task 4.1: Add slot for creation failure

**Files:**
- Modify: `src/mainwindow.h`

- [ ] **Step 1: Add slot declaration**

Add after `onDailyReportCreated`:

```cpp
void onDailyReportCreateFailed(const QString& error);
```

- [ ] **Step 2: Commit**

```bash
git add src/mainwindow.h
git commit -m "feat: add onDailyReportCreateFailed slot"
```

---

### Task 4.2: Implement create failure handler

**Files:**
- Modify: `src/mainwindow.cpp`

- [ ] **Step 1: Add implementation**

Add after `onDailyReportCreated`:

```cpp
void MainWindow::onDailyReportCreateFailed(const QString& error) {
    qDebug() << "日报创建失败:" << error;

    // Check if it's "already exists" error
    if (error.contains("已存在") || error.contains("重复")) {
        qDebug() << "日报已存在，重新获取列表...";
        apiManager->getDailyReportList(1, 50);
    } else {
        // Other error, show to user
        QMessageBox::warning(this, "同步失败", error);
    }
}
```

- [ ] **Step 2: Commit**

```bash
git add src/mainwindow.cpp
git commit -m "feat: implement create failure handler"
```

---

### Task 4.3: Connect the failure signal

**Files:**
- Modify: `src/mainwindow.cpp:setupApiConnections()`

- [ ] **Step 1: Connect signal**

Add after `connect(apiManager, &ApiManager::dailyReportCreated, ...)`:

```cpp
connect(apiManager, &ApiManager::dailyReportCreateFailed, this, &MainWindow::onDailyReportCreateFailed);
```

- [ ] **Step 2: Build and verify**

```bash
cd /root/DailyReport/build && make -j$(nproc)
```

Expected: No errors

- [ ] **Step 3: Commit**

```bash
git add src/mainwindow.cpp
git commit -m "feat: connect dailyReportCreateFailed signal"
```

---

## Chunk 5: Test

### Task 5.1: Build all tests

- [ ] **Step 1: Clean build**

```bash
cd /root/DailyReport/build && rm -rf * && cmake .. && make -j$(nproc)
```

Expected: No errors

- [ ] **Step 2: Run unit tests**

```bash
./tests/test_dailyreport
```

Expected: All 20 tests pass

- [ ] **Step 3: Run E2E tests**

```bash
./tests/test_e2e
```

Expected: All 14 tests pass

- [ ] **Step 4: Commit**

```bash
git add -A && git commit -m "test: all tests pass after sync fix"
```

---

## Verification Checklist

| Check | Status |
|-------|--------|
| Build succeeds without errors | [ ] |
| Unit tests pass (20/20) | [ ] |
| E2E tests pass (14/14) | [ ] |
| month/week correctly computed | [ ] |
| Error handling for duplicates | [ ] |
| Retry logic works | [ ] |

---

## Rollback Plan

If issues occur:
```bash
git log --oneline
git revert <commit-hash> --no-edit
```

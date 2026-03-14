# 工时记录同步逻辑重构设计文档

**日期:** 2026-03-14
**作者:** Claude Opus 4.6
**状态:** 待实施

---

## 1. 背景与问题

当前的同步逻辑存在以下问题：

1. **登录时自动创建日报**：用户登录成功后，`onDailyReportListReceived` 会立即检查今日日报是否存在，如果不存在则创建。这违背了"用户主动同步"的预期。

2. **创建逻辑分散**：`createTodayDailyReportIfNotExist()` 在多个地方被调用（`loadTodaySessions`、`onDailyReportListReceived`），逻辑分散不易维护。

3. **同步流程不清晰**：
   - 登录时获取日报列表后会自动创建
   - 同步按钮点击时又会重新检查日报是否存在
   - 存在不必要的重复检查和创建尝试

4. **错误处理复杂**：`onDailyReportCreateFailed` 中有重试逻辑（静态变量 `hasRetried`），增加了复杂度。

---

## 2. 设计目标

1. **同步时机明确**：今日日报的创建/获取仅在用户点击"同步"按钮时进行。

2. **登录行为简化**：登录成功后仅加载历史日报数据，不进行创建操作。

3. **逻辑集中**：将所有日报相关的操作集中到 `CloudSessionManager::syncToday()` 及其回调中。

4. **错误处理简化**：创建失败直接终止同步流程并提示用户，移除重试逻辑。

---

## 3. 核心改动

### 3.1 CloudSessionManager 修改

#### 新增方法

```cpp
// cloudsessionmanager.h
public:
    // 创建今日日报（无条件创建，不检查是否存在）
    void createTodayDailyReport();

    // 检查并同步今日日报（在 getDailyReportList 回调中调用）
    // @param reports 日报列表
    void checkAndSyncTodayDailyReport(const QJsonArray& reports);
```

#### 修改方法

**`loadTodaySessions()` - 移除自动创建逻辑**

修改前：
```cpp
void CloudSessionManager::loadTodaySessions() {
    QString today = getCurrentDate();
    if (todayDailyReportUuid.isEmpty()) {
        qDebug() << "今日日报UUID未设置，尝试创建日报...";
        createTodayDailyReportIfNotExist();  // ← 移除这行
        return;
    }
    // ...
}
```

修改后：
```cpp
void CloudSessionManager::loadTodaySessions() {
    QString today = getCurrentDate();
    if (todayDailyReportUuid.isEmpty()) {
        qDebug() << "今日日报UUID未设置，无法加载详情";
        return;  // 直接返回，不创建
    }
    // ...
}
```

**`syncToday()` - 保持现有逻辑（已满足需求）**

```cpp
void CloudSessionManager::syncToday() {
    QString today = getCurrentDate();
    if (!buffer.contains(today) || buffer[today].isEmpty()) {
        qDebug() << "今天没有会话需要同步";
        return;
    }

    // UUID 未设置时调用 getDailyReportList
    if (todayDailyReportUuid.isEmpty()) {
        qDebug() << "日报UUID未设置，检查今日日报是否存在...";
        ApiManager::instance().getDailyReportList(1, 50);
        return;
    }

    // UUID 已设置，直接同步
    qDebug() << "使用已设置的日报UUID同步:" << todayDailyReportUuid;
    syncDailyReport(todayDailyReportUuid, today);
}
```

#### 修改 `createTodayDailyReportIfNotExist()`

修改为public方法 `createTodayDailyReport()`，删除"ifNotExist"检查：

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

---

### 3.2 MainWindow 修改

#### `onLoginClicked()` - 仅加载历史数据

**修改前：**
```cpp
connect(dialog, &LoginDialog::loginCompleted, this, [this](const QString& token) {
    statusLabel->setText("状态: 登录成功");
    CloudSessionManager::instance().setApplicantInfo("SQ13793", "孔启楠");
    apiManager->getDailyReportList(1, 50);  // ← 只需这行
});
```

**修改后：** 无需修改（当前逻辑已正确，但需要移除后续的自动创建行为）

#### `onSync()` - 保持现有逻辑

```cpp
void MainWindow::onSync() {
    QString today = getCurrentDate();
    CloudSessionManager& mgr = CloudSessionManager::instance();

    if (!mgr.getBuffer().contains(today) || mgr.getBuffer()[today].isEmpty()) {
        QMessageBox::information(this, "同步", "今天没有记录需要同步");
        return;
    }

    statusLabel->setText("状态: 同步中...");
    mgr.syncToday();  // 保持现有逻辑
}
```

#### `onDailyReportListReceived()` - 简化逻辑

**修改前：**
```cpp
void MainWindow::onDailyReportListReceived(const QJsonArray& reports) {
    QString today = getCurrentDate();
    CloudSessionManager& mgr = CloudSessionManager::instance();
    bool hasLocalSessions = mgr.getSessionCount(today) > 0;

    if (reports.isEmpty()) {
        if (hasLocalSessions) {
            mgr.createTodayDailyReportIfNotExist();  // ← 移除
        }
        return;
    }

    bool foundTodayReport = false;
    for (const QJsonValue& value : reports) {
        QJsonObject report = value.toObject();
        if (report["dailyReportDate"].toString() == today) {
            mgr.setTodayDailyReport(...);
            foundTodayReport = true;
        }
    }

    if (!foundTodayReport) {
        mgr.createTodayDailyReportIfNotExist();  // ← 移除
        return;
    }

    mgr.loadRecentDaysSessions(3);
    mgr.loadTodaySessions();
    mgr.syncToday();
}
```

**修改后：**
```cpp
void MainWindow::onDailyReportListReceived(const QJsonArray& reports) {
    qDebug() << "获取日报列表成功，共" << reports.size() << "条记录";

    QString today = getCurrentDate();
    CloudSessionManager& mgr = CloudSessionManager::instance();

    // 解析所有日报详情（用于加载历史数据）
    for (const QJsonValue& value : reports) {
        QJsonObject report = value.toObject();
        QString date = report["dailyReportDate"].toString();
        mgr.loadDailyReportDetails(date);
    }

    // 检查今日日报是否已由 syncToday() 处理
    if (mgr.getTodayDailyReportUuid().isEmpty()) {
        // UUID 未设置，说明 syncToday() 还未设置
        // 检查今日日报是否存在
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

    // 加载最近3天的日报详情（如果未加载）
    mgr.loadRecentDaysSessions(3);
}
```

**说明：**
- 移除了所有 `createTodayDailyReportIfNotExist()` 调用
- 移除了 `onDailyReportCreated()` 中的自动同步逻辑
- 移除了 `onDailyReportCreateFailed()` 中的重试逻辑

#### `onDailyReportCreated()` - 简化回调处理

**修改前：**
```cpp
void MainWindow::onDailyReportCreated(...) {
    // 复杂的 UUID 提取逻辑...

    // 如果无法提取 UUID，调用 getDailyReportList（可能造成重复创建）
    apiManager->getDailyReportList(1, 50);
}
```

**修改后：**
```cpp
void MainWindow::onDailyReportCreated(const QString& message, const QString& status) {
    if (status != "创建成功") {
        // 失败处理
        QMessageBox::warning(this, "同步失败", message);
        return;
    }

    // 成功：尝试提取 UUID
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

    // 无法提取 UUID，提示用户
    qDebug() << "创建成功但未返回UUID，无法继续同步";
    QMessageBox::warning(this, "同步失败", "创建日报成功但未返回UUID，请重试");
}
```

#### `onDailyReportCreateFailed()` - 简化失败处理

**修改前：**
```cpp
void MainWindow::onDailyReportCreateFailed(const QString& error) {
    if (error.contains("已存在") || error.contains("重复")) {
        // 有重试逻辑...
        static bool hasRetried = false;
        if (!hasRetried) {
            hasRetried = true;
            apiManager->getDailyReportList(1, 50);
        } else {
            CloudSessionManager::instance().clearBuffer();
            hasRetried = false;
        }
    } else {
        QMessageBox::warning(this, "同步失败", error);
    }
}
```

**修改后：**
```cpp
void MainWindow::onDailyReportCreateFailed(const QString& error) {
    qDebug() << "日报创建失败:" << error;
    QMessageBox::warning(this, "同步失败", error);
}
```

---

### 3.3 移除 `MainWindow` 成员变量

```cpp
// 移除 hasTriedCreateTodayReport（不再需要）
// bool hasTriedCreateTodayReport = false;
```

---

## 4. 数据流

### 登录流程

```
用户点击登录
    ↓
LoginDialog 显示并等待输入
    ↓
登录成功 → setApplicantInfo()
    ↓
apiManager.getDailyReportList(1, 50)  // 获取最近50条
    ↓
onDailyReportListReceived(reports)
    ↓
遍历 reports，对每个日报调用 loadDailyReportDetails(date)
    ↓
parseDailyReportDetails(tasks, date) → buffer[date] = tasks
```

### 同步流程（首次同步，今日日报不存在）

```
用户点击同步
    ↓
onSync() → syncToday()
    ↓
[今天有本地会话?]
    ├─ 否 → 提示"No sessions to sync"，结束
    └─ 是
        ↓
    [todayDailyReportUuid 已设置?]
        ├─ 是 → syncDailyReport()，结束
        └─ 否
            ↓
        apiManager.getDailyReportList(1, 50)
            ↓
        onDailyReportListReceived(reports)
            ↓
        [找到今日日报?]
            ├─ 是 → setTodayDailyReport(uuid, month, week) → syncToday()
            └─ 否
                ↓
            CloudSessionManager.createTodayDailyReport()
                ↓
            ApiManager.createDailyReport()
                ↓
            onDailyReportCreated(message, status)
                ↓
            [创建成功?]
                ├─ 是 → setTodayDailyReport() → syncDailyReport()
                └─ 否 → QMessageBox::warning("同步失败")
```

### 同步流程（已存在今日日报）

```
用户点击同步
    ↓
onSync() → syncToday()
    ↓
[今天有本地会话?]
    ├─ 否 → 提示"No sessions to sync"，结束
    └─ 是
        ↓
    [todayDailyReportUuid 已设置?]
        ├─ 是 → syncDailyReport(uuid, today)，结束
        └─ 否
            ↓
        apiManager.getDailyReportList(1, 50)
            ↓
        onDailyReportListReceived(reports)
            ↓
        [找到今日日报?]
            ├─ 是 → setTodayDailyReport(uuid, month, week)
                   → syncDailyReport(uuid, today)，结束
            └─ 否 → 创建日报（见上一流程）
```

---

## 5. 错误处理

| 场景 | 错误码 | 处理方式 |
|------|--------|---------|
| 登录失败 | 401 | 提示"用户名/密码/验证码错误" |
| 获取日报列表失败 | 401/500 | 提示相应错误信息 |
| 创建日报失败 | 400/401/500 | 提示错误信息，终止同步 |
| 创建日报失败（已存在） | 300 | 提示错误信息，终止同步 |
| 同步日报失败 | 404 | 提示"日报不存在" |
| 同步日报失败 | 500 | 提示服务器错误 |

---

## 6. 测试场景

### 单元测试

1. **CloudSessionManager::createTodayDailyReport()**
   - 测试创建请求参数正确性
   - 测试 month/week 计算正确性

2. **CloudSessionManager::syncToday()**
   - 测试无本地会话时的行为
   - 测试 UUID 已设置时的行为
   - 测试 UUID 未设置时调用 getDailyReportList

3. **CloudSessionManager::checkAndSyncTodayDailyReport()**
   - 测试找到今日日报时设置 UUID
   - 测试未找到今日日报时不处理

### 集成测试

1. **首次同步（今日日报不存在）**
   - 准备：清空 buffer，添加今日会话
   - 操作：点击同步
   - 期望：创建今日日报 → 设置 UUID → 同步成功

2. **已存在今日日报的同步**
   - 准备：手动创建今日日报，清空 buffer，添加今日会话
   - 操作：点击同步
   - 期望：设置 UUID → 同步成功

3. **登录后加载历史数据**
   - 准备：登录成功
   - 期望：获取最近3天日报详情，buffer 中有数据

4. **同步失败处理**
   - 准备：断开网络连接
   - 操作：点击同步
   - 期望：提示错误，不造成死循环

---

## 7. 文件修改清单

| 文件 | 修改内容 |
|------|---------|
| `src/cloudsessionmanager.h` | 新增 `createTodayDailyReport()` 和 `checkAndSyncTodayDailyReport()` 方法声明 |
| `src/cloudsessionmanager.cpp` | 修改 `loadTodaySessions()` 移除自动创建；修改 `createTodayDailyReportIfNotExist()` 为 `createTodayDailyReport()`；新增 `checkAndSyncTodayDailyReport()` |
| `src/mainwindow.h` | 移除 `hasTriedCreateTodayReport` 成员变量 |
| `src/mainwindow.cpp` | 简化 `onDailyReportListReceived()`；简化 `onDailyReportCreated()`；简化 `onDailyReportCreateFailed()` |

---

## 8. 风险与注意事项

1. **现有用户升级**：已存在本地会话但未同步的用户，首次点击同步会创建今日日报

2. **并发同步**：如果用户快速点击同步按钮多次，可能会触发多次创建尝试。建议在 `syncToday()` 开始时设置一个 flag 防止并发。

3. **API 响应格式**：确保创建日报的 API 响应包含 `uuid` 字段，否则无法继续同步。

---

## 9. 后续优化（可选）

1. **添加同步进度提示**：在状态栏显示"同步中..."、"创建日报中..."等。

2. **添加同步按钮状态管理**：同步时禁用按钮，防止重复点击。

3. **添加本地会话持久化**：在同步失败时，确保本地会话不丢失。

4. **跨设备同步**：考虑从云端拉取最近的会话数据，与本地合并。

---

## 10. 实施检查清单

- [ ] 修改 `cloudsessionmanager.h` 添加新方法声明
- [ ] 修改 `cloudsessionmanager.cpp` 实现新方法和修改现有方法
- [ ] 修改 `mainwindow.h` 移除 `hasTriedCreateTodayReport`
- [ ] 修改 `mainwindow.cpp` 简化回调函数
- [ ] 运行单元测试验证逻辑
- [ ] 手动测试同步流程
- [ ] 测试登录后加载历史数据
- [ ] 测试同步失败处理
- [ ] 提交代码

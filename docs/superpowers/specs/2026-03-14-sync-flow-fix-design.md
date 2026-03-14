# 同步流程修复设计文档

**日期:** 2026-03-14
**状态:** 已批准
**作者:** Claude Code

---

## 问题描述

当前代码在用户登录后会自动创建/同步今日日报，而不是等待用户点击"同步"按钮。

### 具体表现

1. **登录后云端无今日日报** → 自动创建今日日报（错误）
2. **登录后云端有今日日报** → 自动同步本地会话到云端（错误）

### 期望行为

1. **登录后云端无今日日报** → 只加载历史数据，不创建
2. **登录后云端有今日日报** → 加载并展示，不同步
3. **点击同步后云端无今日日报** → 创建 → 获取列表 → 同步
4. **点击同步后云端有今日日报** → 找到 UUID → 同步

---

## 架构设计

### 核心思路

添加状态标志 `m_isSyncRequest` 区分 `getDailyReportList()` 的调用来源：

| 调用来源 | `m_isSyncRequest` | 行为 |
|---------|------------------|------|
| 登录后自动获取 | `false` | 只记录 UUID，加载历史数据 |
| 用户点击同步 | `true` | 找到→同步，未找到→创建→再获取→同步 |

### 数据流

```
┌─────────────────────────────────────────────────────────────────┐
│                        用户登录                                  │
│                           ↓                                      │
│              apiManager->getDailyReportList(1, 50)               │
│                           ↓                                      │
│              onDailyReportListReceived(reports)                  │
│                           ↓                                      │
│         ┌─────────────────┴─────────────────┐                    │
│         ↓                                   ↓                    │
│   遍历加载历史数据                      查找今日日报               │
│   loadDailyReportDetails(date)          foundToday = true/false  │
│         ↓                                   ↓                    │
│         └─────────────────┬─────────────────┘                    │
│                           ↓                                      │
│              m_isSyncRequest == false                            │
│                           ↓                                      │
│    ┌──────────────────────┴──────────────────────┐               │
│    ↓                                             ↓               │
│ foundToday=true                            foundToday=false      │
│ 记录 UUID                                     不做任何事          │
│ 不同步                                        不创建              │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                      用户点击同步                                │
│                           ↓                                      │
│              m_isSyncRequest = true                              │
│                           ↓                                      │
│         ┌─────────────────┴─────────────────┐                    │
│         ↓                                   ↓                    │
│   UUID 已存在                            UUID 不存在              │
│         ↓                                   ↓                    │
│   syncDailyReport(uuid, today)      getDailyReportList(1, 50)   │
│   m_isSyncRequest = false                 ↓                      │
│                                    onDailyReportListReceived     │
│                                            ↓                     │
│                              m_isSyncRequest == true             │
│                                            ↓                     │
│                              ┌───────────────┴───────────────┐   │
│                              ↓                               ↓   │
│                        foundToday=true                 foundToday=false
│                              ↓                               ↓   │
│                        syncToday()                    createTodayDailyReport()
│                        m_isSyncRequest=false                 ↓   │
│                                                      onDailyReportCreated
│                                                            ↓     │
│                                                      getDailyReportList
│                                                            ↓     │
│                                                      onDailyReportListReceived
│                                                            ↓     │
│                                                      foundToday=true
│                                                            ↓     │
│                                                      syncToday() │
│                                                      m_isSyncRequest=false
└─────────────────────────────────────────────────────────────────┘
```

---

## 组件设计

### 1. MainWindow 成员变量

**新增:**
```cpp
private:
    bool m_isSyncRequest = false;  // 标记当前是否是同步请求
```

### 2. MainWindow::onSync()

**修改内容:**
```cpp
void MainWindow::onSync() {
    QString today = getCurrentDate();
    CloudSessionManager& mgr = CloudSessionManager::instance();

    // 检查本地是否有会话
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

### 3. MainWindow::onDailyReportListReceived()

**修改内容:**
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

### 4. MainWindow::onDailyReportCreated()

**保持不变**，创建成功后调用 `getDailyReportList()` 获取 UUID：
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

---

## 四种情况处理总结

| 情况 | 触发 | 数据流 | 结果 |
|------|------|--------|------|
| **1. 登录后云端无日报** | `onLoginClicked()` → `getDailyReportList()` | `onDailyReportListReceived()` → `m_isSyncRequest=false` → `foundToday=false` | 加载历史数据，不创建 |
| **2. 登录后云端有日报** | `onLoginClicked()` → `getDailyReportList()` | `onDailyReportListReceived()` → `m_isSyncRequest=false` → `foundToday=true` | 记录 UUID，展示数据，不同步 |
| **3. 同步后云端无日报** | `onSync()` → `getDailyReportList()` | `onDailyReportListReceived()` → `m_isSyncRequest=true` → `foundToday=false` → `createTodayDailyReport()` → `onDailyReportCreated()` → `getDailyReportList()` → `onDailyReportListReceived()` → `foundToday=true` → `syncToday()` | 创建 → 同步 |
| **4. 同步后云端有日报** | `onSync()` → UUID 已存在 | `syncDailyReport()` | 直接同步 |
| **4b. 同步后云端有日报（UUID 不存在）** | `onSync()` → `getDailyReportList()` | `onDailyReportListReceived()` → `m_isSyncRequest=true` → `foundToday=true` → `syncToday()` | 找到 → 同步 |

---

## 错误处理

| 错误场景 | 处理方式 |
|---------|---------|
| 创建日报失败 | `onDailyReportCreateFailed()` 显示错误提示，`m_isSyncRequest` 重置为 false |
| 同步失败 | `syncFailed` 信号显示错误提示，`m_isSyncRequest` 重置为 false |
| 网络错误 | API 回调中处理，显示错误提示 |

---

## 测试场景

### 手动测试

1. **登录后云端无今日日报**
   - 清空云端今日日报
   - 登录
   - 验证：不自动创建，UI 提示"今天没有记录"

2. **登录后云端有今日日报**
   - 云端已存在今日日报
   - 登录
   - 验证：UI 展示云端数据，但本地会话不同步

3. **点击同步 - 云端无今日日报**
   - 本地有会话，云端无今日日报
   - 点击同步
   - 验证：创建 → 同步成功

4. **点击同步 - 云端有今日日报**
   - 本地有会话，云端有今日日报
   - 点击同步
   - 验证：同步成功

---

## 文件清单

| 文件 | 修改类型 |
|------|---------|
| `src/mainwindow.h` | 添加 `m_isSyncRequest` 成员变量 |
| `src/mainwindow.cpp` | 修改 `onSync()`, `onDailyReportListReceived()` |

---

## 验收标准

- [ ] 登录后不自动创建今日日报
- [ ] 登录后不自动同步本地会话
- [ ] 登录后能正确展示云端今日日报数据（如果存在）
- [ ] 点击同步能正确处理四种情况
- [ ] 所有单元测试通过

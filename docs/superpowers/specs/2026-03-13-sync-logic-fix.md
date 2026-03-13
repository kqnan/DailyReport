# 修复同步逻辑设计文档

**问题：** 当点击同步按钮时，如果今日已经创建了日报，会同步失败。

**日志示例：**
```
创建今日日报...
申请人: "SQ13793" : "孔启楠"
日期: "2026-03-13"  月份: ""  周: ""
创建日报API调用失败: "2026-03-13已存在日报，请勿重复添加！"
```

**根本原因：**
1. `syncToday()` 在没有 `todayDailyReportUuid` 时会先获取日报列表
2. 如果列表中找到今日日报，会设置 `todayDailyReportUuid` 然后同步
3. 但如果列表为空或未找到今日日报，会尝试创建日报
4. 创建时 `todayMonth` 和 `todayWeek` 为空，导致 API 参数不完整
5. API 返回 "已存在日报" 错误后，没有正确的回退逻辑

**修复目标：** 确保同步流程能正确处理日报已存在的情况，并正确获取 month/week 信息

---

## 架构设计

### 组件关系

```
MainWindow (UI层)
    ↓
CloudSessionManager (业务逻辑层)
    ↓
ApiManager (网络层)
```

### 新增机制

1. **month/week 自动获取**：
   - 从日报列表 API 返回的数据中提取 `month` 和 `week`
   - 使用本地日期计算 `month` (yyyy-MM) 和 `week` (星期X)

2. **创建失败回退逻辑**：
   - 创建日报失败时（statusCode != 200），重新获取日报列表
   - 从列表中提取已存在的日报信息

3. **增加日志**：
   - 详细记录每次 API 调用的参数和响应
   - 标记关键状态转换

---

## 数据流

### 正常同步流程

```
onSync()
    ↓
syncToday()
    ↓
[有 todayDailyReportUuid?]
    ├─ 是 → syncDailyReport(uuid, date)
    └─ 否 → getDailyReportList(1, 50)
                      ↓
               onDailyReportListReceived()
                      ↓
              [找到今日日报?]
                  ├─ 是 → setTodayDailyReport(uuid, month, week)
                         → syncToday() → syncDailyReport()
                  └─ 否 → createTodayDailyReportIfNotExist()
                             ↓
                      [创建成功?]
                          ├─ 是 → setTodayDailyReport() → syncDailyReport()
                          └─ 否 → getDailyReportList() (回退)
```

### 创建失败回退流程

```
createDailyReport() → API 返回 statusCode != 200
                         ↓
              onDailyReportCreated() 收到错误
                         ↓
              重新调用 getDailyReportList(1, 50)
                         ↓
              onDailyReportListReceived() 获取已存在日报
                         ↓
              setTodayDailyReport(uuid, month, week)
                         ↓
              syncDailyReport() 成功同步
```

---

## 关键修改

### 1. CloudSessionManager::createTodayDailyReportIfNotExist()

**修改前：**
```cpp
void CloudSessionManager::createTodayDailyReportIfNotExist() {
    QString today = getCurrentDate();
    if (todayDailyReportUuid.isEmpty()) {
        // todayMonth 和 todayWeek 可能为空
       (ApiManager::instance().createDailyReport(
            applicantId, applicantName, today, todayMonth, todayWeek, tasks
        ));
    }
}
```

**修改后：**
```cpp
void CloudSessionManager::createTodayDailyReportIfNotExist() {
    QString today = getCurrentDate();
    if (todayDailyReportUuid.isEmpty()) {
        // 使用本地计算的 month 和 week
        QString month = today.left(7);  // "2026-03"
        QString week = getCurrentDayOfWeek(today);  // "星期三"

        ApiManager::instance().createDailyReport(
            applicantId, applicantName, today, month, week, tasks
        );
    }
}
```

### 2. MainWindow::onDailyReportCreated()

**修改后：**
```cpp
void MainWindow::onDailyReportCreated(const QString& message, const QString& status) {
    // 尝试解析 UUID...

    // 如果创建失败，重新获取日报列表
    if (status != "创建成功") {
        qDebug() << "创建日报失败，尝试重新获取列表...";
        apiManager->getDailyReportList(1, 50);
        return;
    }

    // 创建成功，设置信息并同步
    // ...
}
```

### 3. ApiManager::createDailyReport() 响应处理

**修改后：**
```cpp
// 在响应处理中
if (statusCode == 200) {
    emit dailyReportCreated(message, "创建成功");
} else {
    // 发送错误信息让调用方处理
    emit dailyReportCreateFailed(obj["message"].toString());
}
```

---

## 错误处理

| 错误场景 | 处理方式 |
|---------|---------|
| 日报已存在 (statusCode=300) | 重新获取列表，使用已存在日报信息 |
| 网络错误 | 显示错误信息给用户 |
| JSON 解析失败 | 记录日志，提示用户重试 |
| 月/周为空 | 使用本地计算的值 |

---

## 测试场景

1. **场景 1：日报已存在，直接同步**
   - 前置：今日已创建日报
   - 操作：点击同步
   - 期望：成功同步，不重复创建

2. **场景 2：日报不存在，创建后同步**
   - 前置：今日未创建日报
   - 操作：点击同步
   - 期望：成功创建并同步

3. **场景 3：创建失败回退**
   - 前置：网络或服务器问题导致创建失败
   - 操作：点击同步
   - 期望：自动回退到获取列表，使用已存在日报

4. **场景 4：month/week 正确传递**
   - 前置：任意
   - 期望：API 调用包含正确的 month (yyyy-MM) 和 week (星期X)

---

## 文件修改清单

| 文件 | 修改内容 |
|------|---------|
| `src/cloudsessionmanager.h` | 添加 `getCurrentDayOfWeek()` 方法声明 |
| `src/cloudsessionmanager.cpp` | 实现 `getCurrentDayOfWeek()`，修改 `createTodayDailyReportIfNotExist()` |
| `src/mainwindow.h` | 修改 `onDailyReportCreated()` 信号签名 |
| `src/mainwindow.cpp` | 修改 `onDailyReportCreated()`，添加回退逻辑 |
| `src/apimanager.h` | 添加 `dailyReportCreateFailed` 信号 |
| `src/apimanager.cpp` | 修改响应处理，发送失败信号 |

---

## 实施顺序

1. 实现 `getCurrentDayOfWeek()` 函数
2. 修改 `createTodayDailyReportIfNotExist()` 使用本地 month/week
3. 添加 `dailyReportCreateFailed` 信号
4. 修改 `onDailyReportCreated()` 处理失败情况
5. 测试所有场景

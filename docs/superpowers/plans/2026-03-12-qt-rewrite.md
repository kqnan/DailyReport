# Qt 工时记录应用重构计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 使用 Qt (C++) 重构现有的 Tauri 工时记录应用，保持所有功能不变

**Architecture:**
- Qt 6.6 (Widgets 框架，跨平台 GUI)
- JSON 文件存储 (按月组织，保持与现有数据格式兼容)
- QJsonDocument (JSON 解析)
- SQLite database (可选，用于更好地管理多段记录和快速查询)
- 单窗口应用，包含主界面、日期选择器、记录列表

**Tech Stack:**
- Qt 6.6 (C++)
- CMake 3.28+
- C++20

**Data Format Compatibility:**
- 保持与现有 Tauri 应用相同的 JSON 文件格式
- 存储路径: 用户数据目录下的 `work_sessions/` 文件夹
- 文件命名: `sessions_YYYY_MM.json`

---

## 文件结构

```
qt-app/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── mainwindow.h/cpp      # Main window with UI
│   ├── sessionmanager.h/cpp  # Session CRUD operations
│   ├── datamodel.h/cpp       # Data models (WorkSession, DailyStatistics)
│   └── utils.h/cpp           # Utilities (formatDuration, etc.)
├── resources/
│   └── icons/
├── CMakeLists.txt
├── package/                  # Packaging scripts
│   ├── windows/
│   ├── linux/
│   └── macos/
└── README.md
```

---

## Task 1: 项目初始化

**Files:**
- Create: `CMakeLists.txt`
- Create: `src/main.cpp`
- Create: `src/mainwindow.h`
- Create: `src/mainwindow.cpp`
- Create: `src/datamodel.h`
- Create: `src/datamodel.cpp`

- [ ] **步骤 1: 创建项目目录结构**

```bash
mkdir qt-app
mkdir -p qt-app/src
mkdir -p qt-app/resources/icons
```

- [ ] **步骤 2: 创建 CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.28)
project(DailyReport VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

add_executable(DailyReport
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/datamodel.h
    src/datamodel.cpp
)

target_link_libraries(DailyReport PRIVATE Qt6::Widgets Qt6::Core)
```

- [ ] **步骤 3: 创建数据模型 `src/datamodel.h`**

```cpp
#pragma once
#include <QString>
#include <QDateTime>
#include <QJsonArray>
#include <optional>

struct WorkSession {
    QString id;
    QString date;           // YYYY-MM-DD
    QString startTime;      // ISO 8601
    QString endTime;        // ISO 8601, optional
    double durationHours;
    QString activity;
    QString workType;

    bool isActive() const { return endTime.isEmpty(); }
    double calculateDuration() const;
};

struct DailyStatistics {
    QString date;
    double totalHours;
    int sessionCount;
};
```

- [ ] **步骤 4: 创建数据模型实现 `src/datamodel.cpp`**

```cpp
#include "datamodel.h"

double WorkSession::calculateDuration() const {
    if (endTime.isEmpty()) return 0.0;

    QDateTime start = QDateTime::fromString(startTime, Qt::ISODate);
    QDateTime end = QDateTime::fromString(endTime, Qt::ISODate);

    double seconds = start.secsTo(end);
    return qRound(seconds / 3600.0 * 10) / 10;
}
```

- [ ] **步骤 5: 创建主窗口 header `src/mainwindow.h`**

```cpp
#pragma once
#include <QMainWindow>
#include "datamodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartShift();
    void onEndShift();
    void onDateChanged(const QDate &date);
    void onEditSession();
    void onDeleteSession();
    void onSaveEdit();

private:
    Ui::MainWindow *ui;
    QList<WorkSession> currentSessions;
    QString currentTimeZone;

    void loadSessions(const QString &date);
    void refreshSessionList();
    QString formatDuration(double hours) const;
};
```

- [ ] **步骤 6: 创建主窗口实现 `src/mainwindow.cpp` (部分)**

```cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect buttons
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::onStartShift);
    connect(ui->endButton, &QPushButton::clicked, this, &MainWindow::onEndShift);
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &MainWindow::onDateChanged);

    // Load today's sessions
    onDateChanged(QDate::currentDate());
}

MainWindow::~MainWindow() {
    delete ui;
}
```

---

## Task 2: 主界面设计

**Files:**
- Create: `resources/mainwindow.ui`
- Create: `src/utils.h`
- Create: `src/utils.cpp`

- [ ] **步骤 1: 创建 UI 文件 `resources/mainwindow.ui`**

使用 Qt Designer 创建以下布局：

```
┌─────────────────────────────────────┐
│        工时记录                      │
├─────────────────────────────────────┤
│ 今日已工作: 0小时    上班片段: 0    │
├─────────────────────────────────────┤
│  [🟢 上班]  (或 [🔴 下班])          │
│  开始时间: 00:00                    │
├─────────────────────────────────────┤
│  [日期选择器] [今天]                │
├─────────────────────────────────────┤
│  [📥 导出 CSV] [📥 导出 JSON]       │
├─────────────────────────────────────┤
│  2026-03-12 的记录 (0 段)           │
│  ┌───────────────────────────────┐ │
│  │ 暂无记录                      │ │
│  │ 点击上方的"上班"按钮开始记录  │ │
│  └───────────────────────────────┘ │
└─────────────────────────────────────┘
```

- [ ] **步骤 2: 创建工具函数 `src/utils.h`**

```cpp
#pragma once
#include <QString>

QString formatDuration(double hours);
QString getStorageDirectory();
QString getFilenameForDate(const QString &date);
QDate parseDateFromFilename(const QString &filename);
```

- [ ] **步骤 3: 创建工具函数实现 `src/utils.cpp`**

```cpp
#include "utils.h"
#include <QCoreApplication>
#include <QStandardPaths>

QString formatDuration(double hours) {
    int h = static_cast<int>(hours);
    int m = static_cast<int>(qRound((hours - h) * 60));

    if (h == 0) return QString::number(m) + "分钟";
    else if (m == 0) return QString::number(h) + "小时";
    else return QString::number(h) + "小时" + QString::number(m) + "分钟";
}

QString getStorageDirectory() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    dir.mkpath(".");
    return dir.filePath("work_sessions");
}
```

- [ ] **步骤 4: 编译 UI 文件**

```bash
cmake -S . -B build
cmake --build build
```

预期: 成功编译，显示主窗口

---

## Task 3: 数据持久化

**Files:**
- Create: `src/sessionmanager.h`
- Create: `src/sessionmanager.cpp`

- [ ] **步骤 1: 创建会话管理器 `src/sessionmanager.h`**

```cpp
#pragma once
#include "datamodel.h"
#include <QString>
#include <QList>
#include <optional>

class SessionManager {
public:
    static SessionManager& instance();

    QList<WorkSession> loadSessions(const QString &date);
    void saveSessions(const QString &date, const QList<WorkSession> &sessions);
    void addSession(const WorkSession &session);
    void updateSession(const QString &date, const WorkSession &session);
    void deleteSession(const QString &date, const QString &sessionId);
    QList<QString> getAvailableDates();
    QList<WorkSession> getTodaySessions();
    WorkSession getActiveSession();
    DailyStatistics getTodayStatistics();

private:
    SessionManager() = default;

    QString getStorageDir() const;
    QString getFilenameForDate(const QString &date) const;
    QList<WorkSession> loadFromFile(const QString &filepath);
    void saveToFile(const QString &filepath, const QList<WorkSession> &sessions);
};
```

- [ ] **步骤 2: 创建会话管理器实现 `src/sessionmanager.cpp` (核心逻辑)**

```cpp
#include "sessionmanager.h"
#include "utils.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QDateTime>
#include <QUuid>

SessionManager& SessionManager::instance() {
    static SessionManager instance;
    return instance;
}

QString SessionManager::getStorageDir() const {
    return getStorageDirectory();
}

QString SessionManager::getFilenameForDate(const QString &date) const {
    // date: "2026-03-12" -> "sessions_2026_03.json"
    QString yearMonth = date.left(7).replace("-", "_");
    return QString("sessions_%1.json").arg(yearMonth);
}

QList<WorkSession> SessionManager::loadSessions(const QString &date) {
    QString filename = getFilenameForDate(date);
    QString filepath = getStorageDir() + "/" + filename;
    return loadFromFile(filepath);
}

QList<WorkSession> SessionManager::loadFromFile(const QString &filepath) {
    QList<WorkSession> sessions;
    QFile file(filepath);

    if (!file.exists()) return sessions;
    if (!file.open(QIODevice::ReadOnly)) return sessions;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isArray()) return sessions;

    QJsonArray array = doc.array();
    for (QJsonValue val : array) {
        if (!val.isObject()) continue;

        QJsonObject obj = val.toObject();
        WorkSession session;
        session.id = obj["id"].toString();
        session.date = obj["date"].toString();
        session.startTime = obj["startTime"].toString();
        session.endTime = obj["endTime"].toString();
        session.durationHours = obj["durationHours"].toDouble();
        session.activity = obj["activity"].toString();
        session.workType = obj["workType"].toString();

        sessions.append(session);
    }

    return sessions;
}

void SessionManager::saveToFile(const QString &filepath, const QList<WorkSession> &sessions) {
    QDir dir(QFileInfo(filepath).path());
    dir.mkpath(".");

    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) return;

    QJsonArray array;
    for (const WorkSession &session : sessions) {
        QJsonObject obj;
        obj["id"] = session.id;
        obj["date"] = session.date;
        obj["startTime"] = session.startTime;
        obj["endTime"] = session.endTime;
        obj["durationHours"] = session.durationHours;
        obj["activity"] = session.activity;
        obj["workType"] = session.workType;
        array.append(obj);
    }

    QJsonDocument doc(array);
    file.write(doc.toJson(QJsonDocument::Indented));
}
```

- [ ] **步骤 3: 实现剩余函数**

```cpp
// sessionmanager.cpp 续

void SessionManager::addSession(const WorkSession &session) {
    QList<WorkSession> sessions = loadSessions(session.date);
    sessions.append(session);
    saveSessions(session.date, sessions);
}

void SessionManager::updateSession(const QString &date, const WorkSession &session) {
    QList<WorkSession> sessions = loadSessions(date);
    for (int i = 0; i < sessions.size(); ++i) {
        if (sessions[i].id == session.id) {
            sessions[i] = session;
            break;
        }
    }
    saveSessions(date, sessions);
}

void SessionManager::deleteSession(const QString &date, const QString &sessionId) {
    QList<WorkSession> sessions = loadSessions(date);
    sessions.erase(std::remove_if(sessions.begin(), sessions.end(),
        [sessionId](const WorkSession &s) { return s.id == sessionId; }), sessions.end());
    saveSessions(date, sessions);
}

QList<QString> SessionManager::getAvailableDates() {
    QDir dir(getStorageDir());
    QList<QString> dates;

    QFileInfoList files = dir.entryInfoList(QStringList("sessions_*.json"), QDir::Files);
    for (const QFileInfo &info : files) {
        QString filename = info.fileName();
        // Extract date from "sessions_2026_03.json"
        QString datePart = filename.remove("sessions_").remove(".json").replace("_", "-");
        // Generate all dates for this month
        QString year = datePart.left(4);
        QString month = datePart.mid(5, 2);
        for (int day = 1; day <= 31; ++day) {
            dates.append(QString("%1-%2-%3").arg(year).arg(month).arg(day, 2, 10, QChar('0')));
        }
    }

    std::sort(dates.begin(), dates.end());
    return dates;
}

QList<WorkSession> SessionManager::getTodaySessions() {
    return loadSessions(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
}

WorkSession SessionManager::getActiveSession() {
    QList<WorkSession> sessions = getTodaySessions();
    for (const WorkSession &s : sessions) {
        if (s.isActive()) return s;
    }
    return WorkSession();
}

DailyStatistics SessionManager::getTodayStatistics() {
    QList<WorkSession> sessions = getTodaySessions();
    DailyStatistics stat;
    stat.date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    stat.totalHours = 0;
    stat.sessionCount = sessions.size();

    for (const WorkSession &s : sessions) {
        stat.totalHours += s.durationHours;
    }

    return stat;
}
```

- [ ] **步骤 4: 测试数据持久化**

```bash
# 构建并运行
cmake --build build
./build/DailyReport
```

操作:
1. 点击"上班"按钮
2. 点击"下班"按钮
3. 关闭应用
4. 重新打开，验证记录是否存在

预期: 记录正确保存和加载

---

## Task 4: 实现上班/下班功能

**Files:**
- Modify: `src/mainwindow.cpp`

- [ ] **步骤 1: 添加成员变量**

```cpp
// mainwindow.h 私有成员添加
private:
    WorkSession *activeSession = nullptr;
```

- [ ] **步骤 2: 实现上班功能**

```cpp
// mainwindow.cpp

void MainWindow::onStartShift() {
    SessionManager &mgr = SessionManager::instance();

    // 检查是否有进行中的记录
    WorkSession existing = mgr.getActiveSession();
    if (!existing.id.isEmpty()) {
        QMessageBox::warning(this, "错误", "已经有进行中的上班记录，请先下班");
        return;
    }

    // 创建新记录
    WorkSession session;
    session.id = QUuid::createUuid().toString().mid(1, 36);
    session.date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    session.startTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    session.endTime = "";
    session.durationHours = 0;
    session.activity = "工作片段";
    session.workType = "默认";

    SessionManager::instance().addSession(session);
    activeSession = new WorkSession(session);

    // 更新 UI
    ui->startButton->hide();
    ui->endButton->show();
    ui->startTimeLabel->setText("开始时间: " + session.startTime.mid(11, 5));
    ui->sessionListWidget->clear();
    loadSessions(session.date);
}

void MainWindow::onEndShift() {
    if (!activeSession) return;

    // 显示结束对话框
    EndDialog dialog(this);
    dialog.setActivity(activeSession->activity);
    dialog.setWorkType(activeSession->workType);

    if (dialog.exec() == QDialog::Accepted) {
        activeSession->endTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        activeSession->durationHours = activeSession->calculateDuration();
        activeSession->activity = dialog.getActivity();
        activeSession->workType = dialog.getWorkType();

        SessionManager::instance().updateSession(activeSession->date, *activeSession);
        delete activeSession;
        activeSession = nullptr;

        ui->endButton->hide();
        ui->startButton->show();
        ui->startTimeLabel->setText("开始时间: -");
        loadSessions(QDate::currentDate().toString("yyyy-MM-dd"));
    }
}
```

- [ ] **步骤 3: 创建结束对话框**

```cpp
// enddialog.h
#pragma once
#include <QDialog>

namespace Ui {
class EndDialog;
}

class EndDialog : public QDialog {
    Q_OBJECT

public:
    explicit EndDialog(QWidget *parent = nullptr);
    QString getActivity() const { return ui->activityEdit->toPlainText(); }
    QString getWorkType() const { return ui->workTypeCombo->currentText(); }
    void setActivity(const QString &activity) { ui->activityEdit->setPlainText(activity); }
    void setWorkType(const QString &workType) { ui->workTypeCombo->setCurrentText(workType); }

private:
    Ui::EndDialog *ui;
};
```

- [ ] **步骤 4: 测试上班/下班流程**

预期:
1. 点击"上班"，按钮变为"下班"
2. 点击"下班"，弹出对话框
3. 输入工作内容后确认
4. 按钮恢复为"上班"
5. 记录正确保存

---

## Task 5: 实现历史记录查询

**Files:**
- Modify: `src/mainwindow.cpp`
- Modify: `resources/mainwindow.ui`

- [ ] **步骤 1: 连接日期选择器信号**

```cpp
// mainwindow.cpp 构造函数中
connect(ui->dateEdit, &QDateEdit::dateChanged, this, &MainWindow::onDateChanged);
```

- [ ] **步骤 2: 实现日期切换**

```cpp
void MainWindow::onDateChanged(const QDate &date) {
    ui->sessionListWidget->clear();
    loadSessions(date.toString("yyyy-MM-dd"));
}

void MainWindow::loadSessions(const QString &date) {
    currentSessions = SessionManager::instance().loadSessions(date);

    if (currentSessions.isEmpty()) {
        ui->sessionListWidget->addItem("暂无记录\n点击上方的\"上班\"按钮开始记录");
        return;
    }

    // 按开始时间排序
    std::sort(currentSessions.begin(), currentSessions.end(),
        [](const WorkSession &a, const WorkSession &b) {
            return a.startTime < b.startTime;
        });

    for (const WorkSession &session : currentSessions) {
        QString timeRange = session.startTime.mid(11, 5) + " - " +
                           (session.endTime.isEmpty() ? "进行中" : session.endTime.mid(11, 5));
        QString text = QString("%1  %2  %3")
            .arg(timeRange)
            .arg(formatDuration(session.durationHours))
            .arg(session.workType);

        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, session.id);
        ui->sessionListWidget->addItem(item);
    }
}
```

- [ ] **步骤 3: 更新 UI 以显示列表**

在 Qt Designer 中添加 QListWidget 到界面

- [ ] **步骤 4: 测试历史记录查询**

预期:
1. 选择不同日期
2. 正确显示该日期的记录
3. 记录按时间排序

---

## Task 6: 实现编辑功能

**Files:**
- Create: `editdialog.h/cpp`
- Modify: `src/mainwindow.cpp`

- [ ] **步骤 1: 创建编辑对话框**

```cpp
// editdialog.h
#pragma once
#include <QDialog>
#include "datamodel.h"

namespace Ui {
class EditDialog;
}

class EditDialog : public QDialog {
    Q_OBJECT

public:
    explicit EditDialog(const WorkSession &session, QWidget *parent = nullptr);
    WorkSession getSession() const;

private:
    Ui::EditDialog *ui;
    WorkSession originalSession;
};
```

```cpp
// editdialog.cpp
#include "editdialog.h"
#include "ui_editdialog.h"

EditDialog::EditDialog(const WorkSession &session, QWidget *parent)
    : QDialog(parent), originalSession(session)
{
    ui = new Ui::EditDialog(this);
    ui->setupUi(this);

    ui->startTimeEdit->setDateTime(QDateTime::fromString(session.startTime, Qt::ISODate));
    ui->startTimeEdit->setReadOnly(true);

    if (!session.endTime.isEmpty()) {
        ui->endTimeEdit->setDateTime(QDateTime::fromString(session.endTime, Qt::ISODate));
        ui->endTimeEdit->setReadOnly(true);
    } else {
        ui->endTimeLabel->setText("进行中");
    }

    ui->activityEdit->setPlainText(session.activity);
    ui->workTypeCombo->setCurrentText(session.workType);
}

WorkSession EditDialog::getSession() const {
    WorkSession session = originalSession;
    session.activity = ui->activityEdit->toPlainText();
    session.workType = ui->workTypeCombo->currentText();
    return session;
}
```

- [ ] **步骤 2: 在主窗口中调用编辑对话框**

```cpp
// mainwindow.cpp

void MainWindow::onEditSession() {
    QListWidgetItem *item = ui->sessionListWidget->currentItem();
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

    EditDialog dialog(*session, this);
    if (dialog.exec() == QDialog::Accepted) {
        WorkSession updated = dialog.getSession();
        SessionManager::instance().updateSession(session->date, updated);
        *session = updated;
        onDateChanged(ui->dateEdit->date());  // Refresh list
    }
}
```

- [ ] **步骤 3: 测试编辑功能**

预期:
1. 双击或点击"编辑"按钮
2. 弹出编辑对话框
3. 修改工作内容和类型
4. 保存后列表更新

---

## Task 7: 实现删除功能

**Files:**
- Modify: `src/mainwindow.cpp`

- [ ] **步骤 1: 实现删除功能**

```cpp
void MainWindow::onDeleteSession() {
    QListWidgetItem *item = ui->sessionListWidget->currentItem();
    if (!item) return;

    QString id = item->data(Qt::UserRole).toString();

    int ret = QMessageBox::question(this, "确认删除",
        QString("确定要删除 %1 的记录吗？").arg(item->text().split(" ")[0]));

    if (ret == QMessageBox::Yes) {
        QString date = ui->dateEdit->date().toString("yyyy-MM-dd");
        SessionManager::instance().deleteSession(date, id);

        // Find and delete from currentSessions
        for (int i = 0; i < currentSessions.size(); ++i) {
            if (currentSessions[i].id == id) {
                currentSessions.removeAt(i);
                break;
            }
        }

        onDateChanged(ui->dateEdit->date());  // Refresh list
    }
}
```

- [ ] **步骤 2: 测试删除功能**

预期:
1. 选择一条记录
2. 点击"删除"按钮
3. 确认对话框出现
4. 删除后列表更新

---

## Task 8: 实现导出功能

**Files:**
- Create: `exportdialog.h/cpp`
- Modify: `src/mainwindow.cpp`

- [ ] **步骤 1: 创建导出对话框**

```cpp
// exportdialog.h
#pragma once
#include <QDialog>

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    QString getFormat() const { return ui->formatCombo->currentText(); }

private:
    Ui::ExportDialog *ui;
};
```

- [ ] **步骤 2: 实现导出逻辑**

```cpp
void MainWindow::onExport() {
    ExportDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) return;

    QString format = dialog.getFormat();
    QString date = ui->dateEdit->date().toString("yyyy-MM-dd");
    QList<WorkSession> sessions = SessionManager::instance().loadSessions(date);

    QString filePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    filePath += "/工时记录_" + date + "." + format.toLower();

    if (format == "JSON") {
        // Export JSON
        QJsonArray array;
        for (const WorkSession &s : sessions) {
            QJsonObject obj;
            obj["日期"] = s.date;
            obj["开始时间"] = s.startTime;
            obj["结束时间"] = s.endTime;
            obj["时长"] = s.durationHours;
            obj["工作类型"] = s.workType;
            obj["工作内容"] = s.activity;
            array.append(obj);
        }

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
            file.close();
            QMessageBox::information(this, "成功", "导出成功: " + filePath);
        }
    } else if (format == "CSV") {
        // Export CSV
        QString csv = "日期,开始时间,结束时间,时长(小时),工作类型,工作内容\n";
        for (const WorkSession &s : sessions) {
            QString endTime = s.endTime.isEmpty() ? "进行中" : s.endTime;
            csv += QString("%1,%2,%3,%4,%5,\"%6\"\n")
                .arg(s.date)
                .arg(s.startTime)
                .arg(endTime)
                .arg(s.durationHours)
                .arg(s.workType)
                .arg(s.activity);
        }

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(csv.toUtf8());
            file.close();
            QMessageBox::information(this, "成功", "导出成功: " + filePath);
        }
    }
}
```

- [ ] **步骤 3: 测试导出功能**

预期:
1. 点击"导出 CSV"或"导出 JSON"
2. 文件保存到Documents目录
3. 文件格式正确

---

## Task 9: 应用关闭时自动保存

**Files:**
- Modify: `src/mainwindow.cpp`

- [ ] **步骤 1: 重写 closeEvent**

```cpp
// mainwindow.cpp

void MainWindow::closeEvent(QCloseEvent *event) {
    if (activeSession) {
        // 自动结束进行中的记录
        activeSession->endTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        activeSession->durationHours = activeSession->calculateDuration();
        activeSession->activity = "工作片段";  // 默认值

        SessionManager::instance().updateSession(activeSession->date, *activeSession);
        delete activeSession;
        activeSession = nullptr;
    }

    QMainWindow::closeEvent(event);
}
```

- [ ] **步骤 2: 测试应用关闭**

预期:
1. 开始上班后直接关闭窗口
2. 重新打开应用查看，记录已自动结束

---

## Task 10: 打包和分发

**Files:**
- Create: `package/linux/deploy.sh`
- Create: `package/windows/deploy.ps1`
- Create: `package/macos/README.md`

- [ ] **步骤 1: Linux 打包脚本**

```bash
# package/linux/deploy.sh
#!/bin/bash

APP_NAME="DailyReport"
APP_DIR="package/linux/${APP_NAME}"

mkdir -p "${APP_DIR}"

# Copy binary
cp -f "build/${APP_NAME}" "${APP_DIR}/"

# Copy Qt plugins
mkdir -p "${APP_DIR}/plugins"
cp -rf "$(qprint --qt-targets | grep -o '/.*qt6.*' | head -1) plugins"/* "${APP_DIR}/plugins/"

# Create desktop file
cat > "${APP_DIR}/${APP_NAME}.desktop" << EOF
[Desktop Entry]
Name=${APP_NAME}
Exec=${APP_NAME}
Type=Application
Categories=Utility;
EOF

# Create AppImage (optional)
# 使用 appimagetool
```

- [ ] **步骤 2: Windows 打包 (使用 windeployqt)**

```powershell
# package/windows/deploy.ps1
$AppDir = "package/windows/DailyReport"
New-Item -ItemType Directory -Path $AppDir -Force

Copy-Item "build\DailyReport.exe" -Destination $AppDir

& "C:\Qt\6.6\...\bin\windeployqt.exe" --dir $AppDir $AppDir\DailyReport.exe
```

- [ ] **步骤 3: macOS 打包**

```bash
# package/macos/deploy.sh
APP_NAME="DailyReport"
APP_DIR="build/${APP_NAME}.app"

# Create app bundle structure
mkdir -p "${APP_DIR}/Contents/MacOS"
mkdir -p "${APP_DIR}/Contents/Frameworks"
mkdir -p "${APP_DIR}/Contents/Plugins"

# Copy binary
cp -f "build/${APP_NAME}" "${APP_DIR}/Contents/MacOS/"

# Copy Qt frameworks
cp -rf /opt/Qt/6.6/.../QtCore.framework "${APP_DIR}/Contents/Frameworks/"
# ... other frameworks

# codesign (optional)
```

- [ ] **步骤 4: 测试安装和运行**

在三个平台分别测试安装包
预期: 应用正常启动，功能正常

---

## 任务完成检查清单

- [ ] 所有代码编写完成
- [ ] Linux 编译测试通过: `cmake --build build`
- [ ] Windows 编译测试通过
- [ ] macOS 编译测试通过
- [ ] 功能测试通过 (上班/下班/编辑/删除/导出)
- [ ] 数据持久化测试通过
- [ ] 跨平台测试通过

---

## 执行指导

Plan complete and saved to `docs/superpowers/plans/2026-03-12-qt-rewrite.md`. Ready to execute?
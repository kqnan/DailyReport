#include "mainwindow.h"
#include "utils.h"
#include "logindialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextStream>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QDesktopServices>
#include <QUrl>
#include <QVBoxLayout>
#include <QNetworkAccessManager>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , activeSession(nullptr)
    , apiManager(&ApiManager::instance())
{
    initUI();
    loadSessions(getCurrentDate());
    setupApiConnections();
}

MainWindow::~MainWindow() {
    delete activeSession;
}

void MainWindow::setupApiConnections() {
    connect(apiManager, &ApiManager::dailyReportListReceived, this, &MainWindow::onDailyReportListReceived);
    connect(apiManager, &ApiManager::dailyReportDetailsReceived, this, [this](const QJsonArray& tasks) {
        // Handle daily report details
        qDebug() << "Received" << tasks.size() << "tasks";
    });
}

void MainWindow::initUI() {
    // Set window properties
    setWindowTitle("工时记录");
    setMinimumSize(600, 500);

    // Create widgets
    startButton = new QPushButton("🟢 上班");
    endButton = new QPushButton("🔴 下班");
    endButton->hide();

    editButton = new QPushButton("编辑");
    deleteButton = new QPushButton("删除");
    exportCsvButton = new QPushButton("📥 导出 CSV");
    exportJsonButton = new QPushButton("📥 导出 JSON");
    todayButton = new QPushButton("今天");
    openFolderButton = new QPushButton("📂 打开记录文件夹");
    loginButton = new QPushButton("🔐 登录");

    statusLabel = new QLabel("状态: 未登录");

    totalHoursLabel = new QLabel("0小时");
    sessionCountLabel = new QLabel("0 段");
    startTimeLabel = new QLabel("开始时间: -");

    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);

    sessionListWidget = new QListWidget();

    // Styling
    startButton->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #10b981, stop:1 #059669); color: white; padding: 16px 48px; font-size: 20px; border-radius: 12px;");
    endButton->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ef4444, stop:1 #dc2626); color: white; padding: 16px 48px; font-size: 20px; border-radius: 12px;");

    // Layout setup
    QWidget *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->addWidget(new QLabel("今日已工作: "));
    headerLayout->addWidget(totalHoursLabel);
    headerLayout->addWidget(new QLabel("上班片段: "));
    headerLayout->addWidget(sessionCountLabel);
    mainLayout->addLayout(headerLayout);

    // Control buttons
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(startButton);
    controlLayout->addWidget(endButton);
    mainLayout->addLayout(controlLayout);

    // Start time label
    mainLayout->addWidget(startTimeLabel);

    // Date selector
    QHBoxLayout *dateLayout = new QHBoxLayout();
    dateLayout->addWidget(new QLabel("选择日期: "));
    dateLayout->addWidget(dateEdit);
    dateLayout->addWidget(todayButton);
    mainLayout->addLayout(dateLayout);

    // Export buttons
    QHBoxLayout *exportLayout = new QHBoxLayout();
    exportLayout->addWidget(exportCsvButton);
    exportLayout->addWidget(exportJsonButton);
    exportLayout->addWidget(openFolderButton);
    mainLayout->addLayout(exportLayout);

    // Status label and login button
    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(loginButton);
    mainLayout->addLayout(statusLayout);

    // Session list
    mainLayout->addWidget(sessionListWidget);

    // Connect signals
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartShift);
    connect(endButton, &QPushButton::clicked, this, &MainWindow::onEndShift);
    connect(dateEdit, &QDateEdit::dateChanged, this, &MainWindow::onDateChanged);
    connect(todayButton, &QPushButton::clicked, [this]() {
        dateEdit->setDate(QDate::currentDate());
        loadSessions(dateEdit->date().toString("yyyy-MM-dd"));
    });
    connect(sessionListWidget, &QListWidget::itemActivated, this, &MainWindow::onEditSession);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::onEditSession);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteSession);
    connect(exportCsvButton, &QPushButton::clicked, this, &MainWindow::onExport);
    connect(exportJsonButton, &QPushButton::clicked, this, &MainWindow::onExport);
    connect(openFolderButton, &QPushButton::clicked, this, &MainWindow::onOpenFolder);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
}

void MainWindow::onStartShift() {
    SessionManager &mgr = SessionManager::instance();

    // Check for existing active session in selected date
    QString selectedDate = dateEdit->date().toString("yyyy-MM-dd");
    WorkSession existing = mgr.getActiveSessionForDate(selectedDate);
    if (!existing.id.isEmpty()) {
        QMessageBox::warning(this, "错误", "已经有进行中的上班记录，请先下班");
        return;
    }

    // Create new session
    WorkSession session;
    session.id = QUuid::createUuid().toString().mid(1, 36);
    session.date = selectedDate;
    session.startTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    session.endTime = "";
    session.durationHours = 0;
    session.activity = "工作片段";
    session.workType = "默认";

    SessionManager::instance().addSession(session);

    // Store active session
    if (activeSession) delete activeSession;
    activeSession = new WorkSession(session);

    // Update UI
    startButton->hide();
    endButton->show();
    startTimeLabel->setText("开始时间: " + session.startTime.mid(11, 5));
    loadSessions(session.date);
}

void MainWindow::onEndShift() {
    if (!activeSession) return;

    // Create end dialog
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("结束上班记录");

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QPlainTextEdit *activityEdit = new QPlainTextEdit(activeSession->activity);
    QComboBox *workTypeCombo = new QComboBox();
    workTypeCombo->addItems({"开发", "会议", "调试", "文档", "其他"});
    workTypeCombo->setCurrentText(activeSession->workType);

    layout->addWidget(new QLabel("工作内容:"));
    layout->addWidget(activityEdit);
    layout->addWidget(new QLabel("工作类型:"));
    layout->addWidget(workTypeCombo);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton("取消");
    QPushButton *confirmBtn = new QPushButton("确认下班");
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(confirmBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(confirmBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    if (dialog->exec() == QDialog::Accepted) {
        activeSession->endTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        activeSession->durationHours = activeSession->calculateDuration();
        activeSession->activity = activityEdit->toPlainText();
        activeSession->workType = workTypeCombo->currentText();

        SessionManager::instance().updateSession(activeSession->date, *activeSession);
        delete activeSession;
        activeSession = nullptr;

        startButton->show();
        endButton->hide();
        startTimeLabel->setText("开始时间: -");
        loadSessions(dateEdit->date().toString("yyyy-MM-dd"));
    }

    delete dialog;
}

void MainWindow::onDateChanged(const QDate &date) {
    loadSessions(date.toString("yyyy-MM-dd"));
}

void MainWindow::loadSessions(const QString &date) {
    currentSessions = SessionManager::instance().loadSessions(date);

    // Update statistics for the selected date
    DailyStatistics stat = SessionManager::instance().getStatisticsForDate(date);
    totalHoursLabel->setText(formatDuration(stat.totalHours));
    sessionCountLabel->setText(QString::number(stat.sessionCount) + " 段");

    sessionListWidget->clear();

    if (currentSessions.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem("暂无记录\n点击上方的\"上班\"按钮开始记录");
        item->setTextAlignment(Qt::AlignCenter);
        sessionListWidget->addItem(item);
        return;
    }

    // Sort by start time
    std::sort(currentSessions.begin(), currentSessions.end(),
        [](const WorkSession &a, const WorkSession &b) {
            return a.startTime < b.startTime;
        });

    for (const WorkSession &session : currentSessions) {
        QString timeRange = session.startTime.mid(11, 5) + " - " +
                           (session.endTime.isEmpty() ? "进行中" : session.endTime.mid(11, 5));

        // Create custom widget with vertical layout to avoid overlapping
        QWidget *itemWidget = new QWidget();
        QVBoxLayout *itemLayout = new QVBoxLayout(itemWidget);
        itemLayout->setContentsMargins(0, 0, 0, 0);
        itemLayout->setSpacing(2);

        // First line: time range, duration, work type
        QString text = QString("%1  %2  %3")
            .arg(timeRange)
            .arg(formatDuration(session.durationHours))
            .arg(session.workType);
        QLabel *timeLabel = new QLabel(text);
        timeLabel->setStyleSheet("font-size: 14px;");

        // Second line: activity description
        QLabel *activityLabel = new QLabel(session.activity != "工作片段" ? session.activity : "无描述");
        activityLabel->setStyleSheet("color: #6b7280; font-size: 12px;");

        itemLayout->addWidget(timeLabel);
        itemLayout->addWidget(activityLabel);
        itemWidget->setLayout(itemLayout);

        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::UserRole, session.id);
        item->setSizeHint(itemWidget->sizeHint());

        sessionListWidget->addItem(item);
        sessionListWidget->setItemWidget(item, itemWidget);
    }
}

QString MainWindow::formatDuration(double hours) const {
    int h = static_cast<int>(hours);
    int m = static_cast<int>(qRound((hours - h) * 60));

    if (h == 0) return QString::number(m) + "分钟";
    else if (m == 0) return QString::number(h) + "小时";
    else return QString::number(h) + "小时" + QString::number(m) + "分钟";
}

DailyStatistics MainWindow::getTodayStatistics() {
    return SessionManager::instance().getTodayStatistics();
}

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

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("编辑记录");

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QDateTime startDt = QDateTime::fromString(session->startTime, Qt::ISODate);
    layout->addWidget(new QLabel("开始时间: " + startDt.toString("yyyy-MM-dd HH:mm")));

    if (!session->endTime.isEmpty()) {
        QDateTime endDt = QDateTime::fromString(session->endTime, Qt::ISODate);
        layout->addWidget(new QLabel("结束时间: " + endDt.toString("yyyy-MM-dd HH:mm")));
    } else {
        layout->addWidget(new QLabel("状态: 进行中"));
    }

    QPlainTextEdit *activityEdit = new QPlainTextEdit(session->activity);
    QComboBox *workTypeCombo = new QComboBox();
    workTypeCombo->addItems({"开发", "会议", "调试", "文档", "其他"});
    workTypeCombo->setCurrentText(session->workType);

    layout->addWidget(new QLabel("工作内容:"));
    layout->addWidget(activityEdit);
    layout->addWidget(new QLabel("工作类型:"));
    layout->addWidget(workTypeCombo);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton("取消");
    QPushButton *saveBtn = new QPushButton("保存");
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    if (dialog->exec() == QDialog::Accepted) {
        session->activity = activityEdit->toPlainText();
        session->workType = workTypeCombo->currentText();

        SessionManager::instance().updateSession(session->date, *session);
        loadSessions(session->date);
    }

    delete dialog;
}

void MainWindow::onDeleteSession() {
    QListWidgetItem *item = sessionListWidget->currentItem();
    if (!item) return;

    QString id = item->data(Qt::UserRole).toString();

    int ret = QMessageBox::question(this, "确认删除",
        QString("确定要删除 %1 的记录吗？").arg(item->text().split(" ")[0]));

    if (ret == QMessageBox::Yes) {
        QString date = dateEdit->date().toString("yyyy-MM-dd");
        SessionManager::instance().deleteSession(date, id);

        // Remove from currentSessions
        for (int i = 0; i < currentSessions.size(); ++i) {
            if (currentSessions[i].id == id) {
                currentSessions.removeAt(i);
                break;
            }
        }

        loadSessions(date);
    }
}

void MainWindow::onExport() {
    QString format = QObject::sender() == exportCsvButton ? "CSV" : "JSON";
    QString date = dateEdit->date().toString("yyyy-MM-dd");
    QList<WorkSession> sessions = SessionManager::instance().loadSessions(date);

    QString filePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    filePath += "/工时记录_" + date + "." + format.toLower();

    if (format == "JSON") {
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
        QString csv = "日期,开始时间,结束时间,时长(小时),工作类型,工作内容\n";
        for (const WorkSession &s : sessions) {
            QString endTime = s.endTime.isEmpty() ? "进行中" : s.endTime;
            QString activity = s.activity;
            csv += QString("%1,%2,%3,%4,%5,\"%6\"\n")
                .arg(s.date)
                .arg(s.startTime)
                .arg(endTime)
                .arg(s.durationHours)
                .arg(s.workType)
                .arg(activity.replace("\"", "\"\""));
        }

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(csv.toUtf8());
            file.close();
            QMessageBox::information(this, "成功", "导出成功: " + filePath);
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (activeSession) {
        // Auto-end active session
        activeSession->endTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        activeSession->durationHours = activeSession->calculateDuration();
        activeSession->activity = "工作片段";

        SessionManager::instance().updateSession(activeSession->date, *activeSession);
        delete activeSession;
        activeSession = nullptr;
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::onOpenFolder() {
    QString folderPath = getStorageDirectory();
    QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}

void MainWindow::onLoginClicked() {
    LoginDialog *dialog = new LoginDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // Get daily report list after successful login
    connect(dialog, &LoginDialog::loginCompleted, this, [this](const QString& token) {
        statusLabel->setText("状态: 登录成功");
        qDebug() << "登录成功，token:" << token;
        apiManager->getDailyReportList(1, 50);
    });

    // Show the dialog
    dialog->show();
}

void MainWindow::onDailyReportListReceived(const QJsonArray& reports) {
    statusLabel->setText("状态: 已登录");
    qDebug() << "获取日报列表成功，共" << reports.size() << "条记录";

    for (const QJsonValue& value : reports) {
        QJsonObject report = value.toObject();
        qDebug() << "日报:" << report["dailyReportDate"].toString()
                 << "创建人:" << report["creator"].toString();
    }
}

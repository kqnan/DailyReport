#include "mainwindow.h"
#include "utils.h"
#include "logindialog.h"
#include "animationutils.h"
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
#include <QPropertyAnimation>
#include <QEasingCurve>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , activeSession(nullptr)
    , apiManager(&ApiManager::instance())
{
    initUI();
    loadSessions(getCurrentDate());
    setupApiConnections();

    // Window fade-in animation
    if (AnimationUtils::animationsEnabled()) {
        setWindowOpacity(0.0);
        QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "windowOpacity");
        fadeIn->setDuration(300);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::InOutQuad);
        fadeIn->start(QPropertyAnimation::DeleteWhenStopped);
    }
}

MainWindow::~MainWindow() {
    delete activeSession;
}

void MainWindow::setupApiConnections() {
    connect(apiManager, &ApiManager::dailyReportListReceived, this, &MainWindow::onDailyReportListReceived);
    connect(apiManager, &ApiManager::dailyReportDetailsReceived, this, [this](const QJsonArray& tasks, const QString& date) {
        // Handle daily report details
        CloudSessionManager::instance().parseDailyReportDetails(tasks, date);
        // Only reload sessions if the received date matches the currently selected date
        QString selectedDate = dateEdit->date().toString("yyyy-MM-dd");
        if (date == selectedDate) {
            loadSessions(date);
        }
    });
    connect(apiManager, &ApiManager::dailyReportCreated, this, &MainWindow::onDailyReportCreated);
    connect(apiManager, &ApiManager::dailyReportCreateFailed, this, &MainWindow::onDailyReportCreateFailed);
    connect(apiManager, &ApiManager::syncSuccess, this, [](const QString& message) {
        qDebug() << "同步成功:" << message;
        QMessageBox::information(nullptr, "同步成功", message);
    });
    connect(apiManager, &ApiManager::syncFailed, this, [](const QString& error) {
        qDebug() << "同步失败:" << error;
        QMessageBox::warning(nullptr, "同步失败", error);
    });
}

void MainWindow::initUI() {
    // Set window properties
    setWindowTitle("工时记录");
    setMinimumSize(600, 500);

    // Morandi color palette QSS styles
    QString morandiStyle = R"(
        QMainWindow, QWidget {
            background-color: #F0EDE8;
            color: #4A4A4A;
            font-family: "Microsoft YaHei", "PingFang SC", sans-serif;
        }
        QLabel {
            background-color: transparent;
            color: #4A4A4A;
            font-size: 14px;
        }
        QDateEdit {
            background-color: #FAF8F5;
            border: 1px solid #D4CFC7;
            border-radius: 6px;
            padding: 6px 12px;
            color: #4A4A4A;
        }
        QDateEdit::drop-down {
            border: none;
        }
        QDateEdit QCalendarWidget {
            background-color: #FAF8F5;
        }
        QListWidget {
            background-color: #FAF8F5;
            border: 1px solid #D4CFC7;
            border-radius: 8px;
            padding: 8px;
            outline: none;
        }
        QListWidget::item {
            background-color: transparent;
            border-radius: 6px;
            padding: 0px;
            margin: 2px 0;
        }
        QListWidget::item:selected {
            background-color: #E8E4DF;
        }
        QListWidget::item:hover {
            background-color: #F5F2EE;
        }
        QPushButton {
            background-color: #E8E4DF;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            color: #4A4A4A;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #D8D4CF;
        }
        QPushButton:pressed {
            background-color: #C8C4BF;
        }
        QComboBox {
            background-color: #FAF8F5;
            border: 1px solid #D4CFC7;
            border-radius: 6px;
            padding: 6px 12px;
            color: #4A4A4A;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox QAbstractItemView {
            background-color: #FAF8F5;
            border: 1px solid #D4CFC7;
            selection-background-color: #E8E4DF;
        }
        QPlainTextEdit {
            background-color: #FAF8F5;
            border: 1px solid #D4CFC7;
            border-radius: 6px;
            padding: 8px;
            color: #4A4A4A;
        }
    )";
    setStyleSheet(morandiStyle);

    // Create widgets with Morandi colors
    startButton = new RippleButton("上班");
    startButton->setStyleSheet(R"(
        RippleButton {
            background-color: #6B8E6B;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 16px 48px;
            font-size: 18px;
            font-weight: bold;
        }
        RippleButton:hover {
            background-color: #5D7D5D;
        }
        RippleButton:pressed {
            background-color: #4F6D4F;
        }
    )");
    startButton->setRippleColor(QColor(255, 255, 255, 80));

    endButton = new RippleButton("下班");
    endButton->setStyleSheet(R"(
        RippleButton {
            background-color: #B87A7A;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 16px 48px;
            font-size: 18px;
            font-weight: bold;
        }
        RippleButton:hover {
            background-color: #A56A6A;
        }
        RippleButton:pressed {
            background-color: #925A5A;
        }
    )");
    endButton->setRippleColor(QColor(255, 255, 255, 80));
    endButton->hide();

    editButton = new QPushButton("编辑");
    editButton->setStyleSheet("background-color: #8B7FA3; color: white; border-radius: 6px; padding: 6px 16px;");

    deleteButton = new QPushButton("删除");
    deleteButton->setStyleSheet("background-color: #B87A7A; color: white; border-radius: 6px; padding: 6px 16px;");

    exportCsvButton = new QPushButton("导出 CSV");
    exportCsvButton->hide();
    exportJsonButton = new QPushButton("导出 JSON");
    exportJsonButton->hide();

    todayButton = new QPushButton("今天");
    todayButton->setStyleSheet("background-color: #8B7FA3; color: white; border-radius: 6px; padding: 6px 16px;");

    openFolderButton = new QPushButton("打开记录文件夹");
    openFolderButton->hide();

    loginButton = new RippleButton("登录");
    loginButton->setStyleSheet(R"(
        RippleButton {
            background-color: #6B8BA3;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            font-size: 13px;
        }
        RippleButton:hover {
            background-color: #5A7A93;
        }
        RippleButton:pressed {
            background-color: #4A6A83;
        }
    )");
    loginButton->setRippleColor(QColor(255, 255, 255, 60));

    syncButton = new RippleButton("同步");
    syncButton->setStyleSheet(R"(
        RippleButton {
            background-color: #6B8BA3;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 13px;
        }
        RippleButton:hover {
            background-color: #5A7A93;
        }
        RippleButton:pressed {
            background-color: #4A6A83;
        }
    )");
    syncButton->setRippleColor(QColor(255, 255, 255, 60));
    syncButton->hide();

    statusLabel = new QLabel("状态: 未登录");

    totalHoursLabel = new QLabel("0小时");
    totalHoursLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #6B8E6B;");

    sessionCountLabel = new QLabel("0 段");
    sessionCountLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #6B8BA3;");

    startTimeLabel = new QLabel("开始时间: -");

    elapsedTimeLabel = new AnimatedLabel();
    elapsedTimeLabel->setText("已工作：0 小时 0 分钟");
    elapsedTimeLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #6B8E6B; padding: 8px;");
    elapsedTimeLabel->hide();  // Initially hidden, shown when shift starts

    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);

    sessionListWidget = new AnimatedListWidget();
    sessionListWidget->setSlideDuration(100);

    // Layout setup
    QWidget *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->addWidget(new QLabel("今日已工作: "));
    headerLayout->addWidget(totalHoursLabel);
    headerLayout->addSpacing(20);
    headerLayout->addWidget(new QLabel("上班片段: "));
    headerLayout->addWidget(sessionCountLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Control buttons
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->addStretch();
    controlLayout->addWidget(startButton);
    controlLayout->addWidget(endButton);
    controlLayout->addStretch();
    mainLayout->addLayout(controlLayout);

    // Start time label
    QHBoxLayout *timeLayout = new QHBoxLayout();
    timeLayout->addStretch();
    timeLayout->addWidget(startTimeLabel);
    timeLayout->addStretch();
    mainLayout->addLayout(timeLayout);

    QHBoxLayout *elapsedLayout = new QHBoxLayout();
    elapsedLayout->addStretch();
    elapsedLayout->addWidget(elapsedTimeLabel);
    elapsedLayout->addStretch();
    mainLayout->addLayout(elapsedLayout);

    // Date selector
    QHBoxLayout *dateLayout = new QHBoxLayout();
    dateLayout->addWidget(new QLabel("选择日期: "));
    dateLayout->addWidget(dateEdit);
    dateLayout->addWidget(todayButton);
    dateLayout->addStretch();
    mainLayout->addLayout(dateLayout);

    // Status label and login button
    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->addWidget(statusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(loginButton);
    mainLayout->addLayout(statusLayout);

    // Record title
    QLabel *recordTitleLabel = new QLabel("当日记录:");
    recordTitleLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 8px;");
    mainLayout->addWidget(recordTitleLabel);

    // Session list
    mainLayout->addWidget(sessionListWidget);

    // Connect signals
    connect(startButton, &RippleButton::clicked, this, &MainWindow::onStartShift);
    connect(endButton, &RippleButton::clicked, this, &MainWindow::onEndShift);
    connect(dateEdit, &QDateEdit::dateChanged, this, &MainWindow::onDateChanged);
    connect(todayButton, &QPushButton::clicked, [this]() {
        dateEdit->setDate(QDate::currentDate());
        loadSessions(dateEdit->date().toString("yyyy-MM-dd"));
    });
    connect(sessionListWidget, &AnimatedListWidget::itemActivated, this, &MainWindow::onEditSession);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::onEditSession);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteSession);
    connect(exportCsvButton, &QPushButton::clicked, this, &MainWindow::onExport);
    connect(exportJsonButton, &QPushButton::clicked, this, &MainWindow::onExport);
    connect(openFolderButton, &QPushButton::clicked, this, &MainWindow::onOpenFolder);
    connect(loginButton, &RippleButton::clicked, this, &MainWindow::onLoginClicked);
    connect(syncButton, &RippleButton::clicked, this, &MainWindow::onSync);

    // Elapsed timer
    elapsedTimer = new QTimer(this);
    connect(elapsedTimer, &QTimer::timeout, this, &MainWindow::updateElapsedTime);
}

void MainWindow::onStartShift() {
    CloudSessionManager &mgr = CloudSessionManager::instance();

    // Check for existing active session for TODAY (new sessions always use today's date)
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    WorkSession existing = mgr.getActiveSessionForDate(today);
    if (!existing.id.isEmpty()) {
        QMessageBox::warning(this, "错误", "已经有进行中的上班记录，请先下班");
        return;
    }

    // Create new session with today's date (independent of calendar selection)
    WorkSession session;
    session.id = QUuid::createUuid().toString().mid(1, 36);
    session.date = today;
    session.startTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    session.endTime = "";
    session.durationHours = 0;
    session.activity = "工作片段";
    session.workType = "默认";

    mgr.addSession(session);

    // Store active session
    if (activeSession) delete activeSession;
    activeSession = new WorkSession(session);

    // Update UI
    startButton->hide();
    endButton->show();
    startTimeLabel->setText("开始时间: " + session.startTime.mid(11, 5));
    elapsedTimeLabel->show();
    elapsedTimeLabel->setTextImmediate("已工作：0 小时 0 分钟");
    elapsedTimer->start(60000);  // 60 seconds = 1 minute
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

        CloudSessionManager::instance().updateSession(*activeSession);
        delete activeSession;
        activeSession = nullptr;

        elapsedTimer->stop();
        elapsedTimeLabel->hide();
        startButton->show();
        endButton->hide();
        startTimeLabel->setText("开始时间: -");
        loadSessions(dateEdit->date().toString("yyyy-MM-dd"));
    }

    delete dialog;
}

void MainWindow::onDateChanged(const QDate &date) {
    QString dateStr = date.toString("yyyy-MM-dd");
    // Load the selected date's daily report details if not already loaded
    CloudSessionManager& mgr = CloudSessionManager::instance();
    if (mgr.getSessionCount(dateStr) == 0) {
        mgr.loadDailyReportDetails(dateStr);
    }
    // Always reload sessions to show the latest data (from buffer or API response)
    loadSessions(dateStr);
}

void MainWindow::loadSessions(const QString &date) {
    currentSessions = CloudSessionManager::instance().getSessions(date);

    // Update statistics for the selected date
    DailyStatistics stat = CloudSessionManager::instance().getStatisticsForDate(date);
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
        itemLayout->setContentsMargins(8, 8, 8, 8);
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

        // Ensure style is applied before calculating sizeHint
        timeLabel->ensurePolished();
        activityLabel->ensurePolished();

        // Calculate proper sizeHint with explicit font metrics
        QFontMetrics fm1(timeLabel->font());
        QFontMetrics fm2(activityLabel->font());
        int lineHeight1 = fm1.height();
        int lineHeight2 = fm2.height();
        int spacing = 2;
        int totalHeight = lineHeight1 + lineHeight2 + spacing;

        // Use the larger of calculated or widget's sizeHint
        QSize hint = itemWidget->sizeHint();
        if (hint.height() < totalHeight) {
            hint.setHeight(totalHeight);
        }

        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::UserRole, session.id);
        item->setSizeHint(hint);

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

// Elapsed time display - stub implementations for Task 1
void MainWindow::updateElapsedTime() {
    if (!activeSession || activeSession->startTime.isEmpty()) return;

    QDateTime start = QDateTime::fromString(activeSession->startTime, Qt::ISODate);
    QDateTime now = QDateTime::currentDateTime();

    if (!start.isValid()) return;

    double seconds = start.secsTo(now);
    if (seconds < 0) seconds = 0;  // Prevent negative

    formatElapsed(seconds);
}

void MainWindow::formatElapsed(double seconds) const {
    if (!elapsedTimeLabel || !activeSession) return;

    int hours = static_cast<int>(seconds / 3600);
    int minutes = static_cast<int>((static_cast<int>(seconds) % 3600) / 60);

    elapsedTimeLabel->setAnimatedText(QString("已工作：%1 小时%2 分钟").arg(hours).arg(minutes));
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

    // 阻止未结束会话编辑
    if (session->endTime.isEmpty()) {
        return;
    }

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
    layout->addWidget(new QLabel("工作时长(小时):"));
    layout->addWidget(durationEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton("取消");
    QPushButton *saveBtn = new QPushButton("保存");
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, dialog, &QDialog::accept);

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
}

void MainWindow::onDeleteSession() {
    QListWidgetItem *item = sessionListWidget->currentItem();
    if (!item) return;

    QString id = item->data(Qt::UserRole).toString();

    int ret = QMessageBox::question(this, "确认删除",
        QString("确定要删除 %1 的记录吗？").arg(item->text().split(" ")[0]));

    if (ret == QMessageBox::Yes) {
        CloudSessionManager::instance().deleteSession(id);

        // Remove from currentSessions
        for (int i = 0; i < currentSessions.size(); ++i) {
            if (currentSessions[i].id == id) {
                currentSessions.removeAt(i);
                break;
            }
        }

        loadSessions(getCurrentDate());
    }
}

void MainWindow::onExport() {
    QString format = QObject::sender() == exportCsvButton ? "CSV" : "JSON";
    QString date = dateEdit->date().toString("yyyy-MM-dd");
    QList<WorkSession> sessions = CloudSessionManager::instance().getSessions(date);

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
        if (elapsedTimer) elapsedTimer->stop();
        activeSession->endTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        activeSession->durationHours = activeSession->calculateDuration();
        activeSession->activity = "工作片段";

        CloudSessionManager::instance().updateSession(*activeSession);
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

        // Set applicant info from LoginDialog's apiManager
        CloudSessionManager::instance().setApplicantInfo("SQ13793", "孔启楠");

        // Get daily report list first
        apiManager->getDailyReportList(1, 50);
    });

    // Show the dialog
    dialog->show();
}

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

void MainWindow::onDailyReportCreated(const QString& message, const QString& status) {
    if (status != "创建成功") {
        qDebug() << "日报创建失败:" << message;
        QMessageBox::warning(this, "同步失败", message);
        return;
    }

    qDebug() << "日报创建成功，获取日报列表以提取 UUID...";
    // 根据 API 文档，创建日报响应不包含 UUID
    // 需要调用获取日报列表 API，遍历日期为今天的日报获取 UUID
    apiManager->getDailyReportList(1, 50);
}


void MainWindow::onDailyReportCreateFailed(const QString& error) {
    qDebug() << "日报创建失败:" << error;
    m_isSyncRequest = false;  // 重置标志
    QMessageBox::warning(this, "同步失败", error);
}

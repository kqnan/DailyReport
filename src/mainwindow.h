#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QDateEdit>
#include <QListWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "datamodel.h"
#include "sessionmanager.h"
#include "apimanager.h"
#include "cloudsessionmanager.h"
#include "ripplebutton.h"
#include "animatedlabel.h"

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
    void onExport();
    void onOpenFolder();
    void onLoginClicked();
    void onDailyReportListReceived(const QJsonArray& reports);
    void onDailyReportCreated(const QString& message, const QString& status);
    void onDailyReportCreateFailed(const QString& error);
    void onSync();
    void setupApiConnections();
    void updateElapsedTime();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    // Sync request flag
    bool m_isSyncRequest = false;

    // Buttons
    RippleButton *startButton;
    RippleButton *endButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    RippleButton *exportCsvButton;
    RippleButton *exportJsonButton;
    RippleButton *todayButton;
    RippleButton *openFolderButton;
    RippleButton *loginButton;
    RippleButton *syncButton;

    // Labels
    QLabel *totalHoursLabel;
    QLabel *sessionCountLabel;
    QLabel *startTimeLabel;
    QLabel *statusLabel;
    AnimatedLabel *elapsedTimeLabel;
    QTimer *elapsedTimer;

    // Date selector
    QDateEdit *dateEdit;
    QListWidget *sessionListWidget;

    // Data
    WorkSession *activeSession;
    QList<WorkSession> currentSessions;

    // API
    ApiManager* apiManager;

    // Helper functions
    void initUI();
    void loadSessions(const QString &date);
    QString formatDuration(double hours) const;
    void formatElapsed(double seconds) const;
    DailyStatistics getTodayStatistics();
};

#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QDateEdit>
#include <QListWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "datamodel.h"
#include "sessionmanager.h"
#include "apimanager.h"
#include "cloudsessionmanager.h"

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

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    // Buttons
    QPushButton *startButton;
    QPushButton *endButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *exportCsvButton;
    QPushButton *exportJsonButton;
    QPushButton *todayButton;
    QPushButton *openFolderButton;
    QPushButton *loginButton;
    QPushButton *syncButton;

    // Labels
    QLabel *totalHoursLabel;
    QLabel *sessionCountLabel;
    QLabel *startTimeLabel;
    QLabel *statusLabel;

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
    DailyStatistics getTodayStatistics();
};

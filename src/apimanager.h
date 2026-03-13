#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QList>
#include <QString>

class ApiManager : public QObject {
    Q_OBJECT

public:
    static ApiManager& instance();

    // Get verification code
    void getVerificationCode();

    // Login with username, password and verification code
    void login(const QString& userNameId, const QString& password, const QString& code);

    // Get daily report list
    void getDailyReportList(int page, int rows);

    // Get daily report details for a specific date
    void getDailyReportDetails(const QString& dailyReportDate, const QString& applicantId);

    // Create daily report for a specific date
    void createDailyReport(const QString& applicantId, const QString& applicantName,
                          const QString& dailyReportDate, const QString& month, const QString& week,
                          const QList<QPair<QString, double>>& tasks);

    // Sync daily report to server
    void syncDailyReport(const QString& uuid, const QString& applicantId, const QString& applicantName,
                        const QString& dailyReportDate, const QString& month, const QString& week,
                        const QList<QPair<QString, double>>& tasks);

    // Get daily report list for a specific date
    void getDailyReportListByDate(const QString& date, int page, int rows);

signals:
    // Verification code signals
    void verificationCodeReceived(int code);
    void verificationCodeFailed(const QString& error);

    // Login signals
    void loginSuccess(const QString& token);
    void loginFailed(const QString& message);

    // Daily report list signals
    void dailyReportListReceived(const QJsonArray& reports);
    void dailyReportListFailed(const QString& error);

    // Daily report details signals
    void dailyReportDetailsReceived(const QJsonArray& tasks, const QString& date);
    void dailyReportDetailsFailed(const QString& error);

    // Sync signals
    void syncSuccess(const QString& message);
    void syncFailed(const QString& error);

private:
    ApiManager() = default;

    QNetworkAccessManager* networkManager = new QNetworkAccessManager(this);
    QString token;
    int verificationCode = 0;

    void handleResponse(QNetworkReply* reply, const QJsonDocument& doc);
    QString getCookieHeader() const;
};

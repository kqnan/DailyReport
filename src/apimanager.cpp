#include "apimanager.h"
#include <QJsonParseError>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>
#include <QByteArray>
#include <QList>
#include <QPair>

ApiManager& ApiManager::instance() {
    static ApiManager instance;
    return instance;
}

void ApiManager::getVerificationCode() {
    QUrl url("https://oa.zhilehuo.com/system/genCodes");

    QNetworkRequest request(url);

    QNetworkReply* reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit verificationCodeFailed(reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        bool ok = false;
        int code = response.toInt(&ok);

        if (ok) {
            verificationCode = code;
            emit verificationCodeReceived(code);
        } else {
            emit verificationCodeFailed("Invalid code format");
        }

        reply->deleteLater();
    });
}

void ApiManager::login(const QString& userNameId, const QString& password, const QString& code) {
    QUrl url("https://oa.zhilehuo.com/system/jwtLogin");
    QJsonDocument doc;
    QJsonObject obj;
    obj["userNameId"] = userNameId;
    obj["password"] = password;
    obj["code"] = code;
    doc.setObject(obj);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager->post(request, doc.toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit loginFailed(reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            emit loginFailed("Invalid JSON response");
            reply->deleteLater();
            return;
        }

        QJsonObject obj = doc.object();
        int statusCode = obj["statusCode"].toInt();

        if (statusCode == 200) {
            QString message = obj["message"].toString();
            // Extract token from message (format: "bearer;eyJ0eXBl...")
            if (message.startsWith("bearer;")) {
                token = message.mid(7); // Skip "bearer;" prefix
                emit loginSuccess(token);
            } else {
                emit loginFailed("Invalid token format");
            }
        } else {
            emit loginFailed(obj["message"].toString());
        }

        reply->deleteLater();
    });
}

void ApiManager::getDailyReportList(int page, int rows) {
    QUrl url("https://oa.zhilehuo.com/office/shiquOaDaily/getPageSet");
    QUrlQuery query;
    query.addQueryItem("page", QString::number(page));
    query.addQueryItem("rows", QString::number(rows));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Cookie", getCookieHeader().toUtf8());

    // Debug: Print request details
    qDebug() << "=== getDailyReportList 请求详情 ===";
    qDebug() << "URL:" << url.toString();
    qDebug() << "请求参数:" << query.query();
    qDebug() << "请求头 Cookie:" << getCookieHeader();
    qDebug() << "==================================";

    QNetworkReply* reply = networkManager->post(request, query.query().toUtf8());
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "=== getDailyReportList 响应详情 ===";
            qDebug() << "错误:" << reply->errorString();
            qDebug() << "==================================";
            emit dailyReportListFailed(reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();

        // Debug: Print response details
        qDebug() << "=== getDailyReportList 响应详情 ===";
        qDebug() << "原始响应:" << response;

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "JSON 解析失败:" << parseError.errorString();
            qDebug() << "==================================";
            emit dailyReportListFailed("Invalid JSON response");
            reply->deleteLater();
            return;
        }

        qDebug() << "格式化响应:" << doc.toJson(QJsonDocument::Indented);
        qDebug() << "==================================";

        QJsonObject obj = doc.object();
        QJsonArray rows = obj["rows"].toArray();
        emit dailyReportListReceived(rows);

        reply->deleteLater();
    });
}

void ApiManager::getDailyReportDetails(const QString& dailyReportDate, const QString& applicantId) {
    QUrl url("https://oa.zhilehuo.com/office/shiquOaDailyTask/getListDay");
    QUrlQuery query;
    query.addQueryItem("dailyReportDate", dailyReportDate);
    query.addQueryItem("applicantId", applicantId);

    QNetworkRequest request(url);
    request.setRawHeader("Cookie", getCookieHeader().toUtf8());

    QNetworkReply* reply = networkManager->post(request, query.query().toUtf8());
    connect(reply, &QNetworkReply::finished, [this, reply, dailyReportDate]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit dailyReportDetailsFailed(reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            emit dailyReportDetailsFailed("Invalid JSON response");
            reply->deleteLater();
            return;
        }

        QJsonArray array = doc.array();
        emit dailyReportDetailsReceived(array, dailyReportDate);

        reply->deleteLater();
    });
}

void ApiManager::createDailyReport(const QString& applicantId, const QString& applicantName,
                                   const QString& dailyReportDate, const QString& month, const QString& week,
                                   const QList<QPair<QString, double>>& tasks) {
    QUrl url("https://oa.zhilehuo.com/office/shiquOaDaily/save");
    QUrlQuery query;
    query.addQueryItem("uuid", "");
    query.addQueryItem("i", QString::number(tasks.size()));
    query.addQueryItem("applicantId", applicantId);
    query.addQueryItem("applicantName", applicantName);
    query.addQueryItem("dailyReportDate", dailyReportDate);
    query.addQueryItem("month", month);
    query.addQueryItem("week", week);

    int serialNumber = 1;
    for (const auto& task : tasks) {
        QString taskKey = QString("task%1").arg(serialNumber);
        QString hoursKey = QString("hours%1").arg(serialNumber);
        query.addQueryItem(taskKey, task.first);
        query.addQueryItem(hoursKey, QString::number(task.second));
        serialNumber++;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Cookie", getCookieHeader().toUtf8());

    qDebug() << "创建日报API调用:";
    qDebug() << "  URL:" << url.toString();
    qDebug() << "  参数:" << query.query();

    QNetworkReply* reply = networkManager->post(request, query.query().toUtf8());
    connect(reply, &QNetworkReply::finished, [this, reply, dailyReportDate]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "创建日报API失败:" << reply->errorString();
            emit dailyReportListFailed(reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);

        // Print raw response
        qDebug() << "创建日报API原始响应:";
        qDebug() << doc.toJson(QJsonDocument::Indented);

        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "创建日报API: 无效的JSON响应";
            emit dailyReportListFailed("Invalid JSON response");
            reply->deleteLater();
            return;
        }

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

        reply->deleteLater();
    });
}

QString ApiManager::getCookieHeader() const {
    if (token.isEmpty()) {
        return "";
    }
    return "token=" + token;
}

void ApiManager::syncDailyReport(const QString& uuid, const QString& applicantId, const QString& applicantName,
                                 const QString& dailyReportDate, const QString& month, const QString& week,
                                 const QList<QPair<QString, double>>& tasks) {
    qDebug() << "同步日报API调用:";
    qDebug() << "  UUID:" << uuid;
    qDebug() << "  申请人:" << applicantId << ":" << applicantName;
    qDebug() << "  日期:" << dailyReportDate;
    qDebug() << "  任务数:" << tasks.size();

    QUrl url("https://oa.zhilehuo.com/office/shiquOaDaily/update");
    QUrlQuery query;
    query.addQueryItem("uuid", uuid);
    query.addQueryItem("i", QString::number(tasks.size()));
    query.addQueryItem("applicantId", applicantId);
    query.addQueryItem("applicantName", applicantName);
    query.addQueryItem("dailyReportDate", dailyReportDate);
    query.addQueryItem("month", month);
    query.addQueryItem("week", week);

    // Add tasks
    int serialNumber = 1;
    for (const auto& task : tasks) {
        QString taskKey = QString("task%1").arg(serialNumber);
        QString hoursKey = QString("hours%1").arg(serialNumber);
        query.addQueryItem(taskKey, task.first);
        query.addQueryItem(hoursKey, QString::number(task.second));
        serialNumber++;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Cookie", getCookieHeader().toUtf8());

    QNetworkReply* reply = networkManager->post(request, query.query().toUtf8());
    connect(reply, &QNetworkReply::finished, [this, reply, uuid]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "同步日报API失败:" << reply->errorString();
            emit syncFailed(reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);

        // Print raw response
        qDebug() << "同步日报API原始响应:";
        qDebug() << doc.toJson(QJsonDocument::Indented);

        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "同步日报API: 无效的JSON响应";
            emit syncFailed("Invalid JSON response");
            reply->deleteLater();
            return;
        }

        QJsonObject obj = doc.object();
        int statusCode = obj["statusCode"].toInt();

        if (statusCode == 200) {
            QString message = obj["message"].toString();
            emit syncSuccess(message);
        } else {
            emit syncFailed(obj["message"].toString());
        }

        reply->deleteLater();
    });
}

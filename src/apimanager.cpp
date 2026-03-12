#include "apimanager.h"
#include <QJsonParseError>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>

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
    url.setQuery(query.query());

    QNetworkRequest request(url);
    request.setRawHeader("Cookie", getCookieHeader().toUtf8());

    QNetworkReply* reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit dailyReportListFailed(reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            emit dailyReportListFailed("Invalid JSON response");
            reply->deleteLater();
            return;
        }

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
    connect(reply, &QNetworkReply::finished, [this, reply]() {
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
        emit dailyReportDetailsReceived(array);

        reply->deleteLater();
    });
}

QString ApiManager::getCookieHeader() const {
    if (token.isEmpty()) {
        return "";
    }
    return "token=" + token;
}

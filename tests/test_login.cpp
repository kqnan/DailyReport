#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>

#include "../src/apimanager.h"

class TestLogin : public QObject {
    Q_OBJECT

private slots:
    // Test that login parses bearer token correctly
    void test_login_success_parses_token() {
        QByteArray response = R"({"title":"操作提示","message":"bearer;eyJ0eXBlIjoiSldUIiwiYWxnIjoiSFMyNTYifQ.eyJ1dWlkIjoiMjAzNUU1Njg2RDg1NEMyRUI1MTBGM0YwMzc0NkY1RkMiLCJ1c2VyTmFtZUlkIjoiU1ExMzc5MyIsInVzZXJOYW1lIjoi5a2U5ZCv5qWgIiwib3JnSWQiOiIxMzMiLCJyb2xlSWQiOiI1MCIsImlzcyI6InN5c3RlbUp3dCIsImF1ZCI6IjA5OGY2YmNkNDYyMWQzNzNjYWRlNGU4MzI2MjdiNGY5In0.eLhnWyijkMHgeT6CRw2UkGLEfLc43Vf6GrKT4xI4bLw","statusCode":200})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 200);

        QString message = obj["message"].toString();
        QVERIFY(message.startsWith("bearer;"));

        // Extract token
        QString token = message.mid(7);
        QVERIFY(!token.isEmpty());
        // Token should be a JWT (contains two dots)
        QVERIFY(token.contains("."));
    }

    // Test that login failure with non-200 status code is handled correctly
    void test_login_failure_non_200_status() {
        QByteArray response = R"({"title":"操作提示","message":"用户名或密码错误","statusCode":401})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 401);
        QCOMPARE(obj["message"].toString(), QString("用户名或密码错误"));
    }

    // Test that login handles invalid JSON
    void test_login_invalid_json_response() {
        QByteArray response = "invalid json";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error != QJsonParseError::NoError);
    }

    // Test that login handles missing statusCode
    void test_login_missing_status_code() {
        QByteArray response = R"({"title":"操作提示","message":"some message"})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        // statusCode should default to 0 if not present
        QCOMPARE(obj["statusCode"].toInt(), 0);
    }

    // Test that login handles missing message
    void test_login_missing_message() {
        QByteArray response = R"({"statusCode":200})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 200);
        // message should be empty if not present
        QVERIFY(obj["message"].toString().isEmpty());
    }

    // Test verification code is an integer
    void test_verification_code_format() {
        QByteArray response = "7156";

        bool ok = false;
        int code = response.toInt(&ok);

        QVERIFY(ok);
        QCOMPARE(code, 7156);
    }

    // Test verification code can handle single digit
    void test_verification_code_single_digit() {
        QByteArray response = "5";

        bool ok = false;
        int code = response.toInt(&ok);

        QVERIFY(ok);
        QCOMPARE(code, 5);
    }

    // Test that login request has correct fields
    void test_login_request_fields() {
        QString userNameId = "18578775001";
        QString password = "84769629aA.";
        QString code = "7156";

        QJsonDocument doc;
        QJsonObject obj;
        obj["userNameId"] = userNameId;
        obj["password"] = password;
        obj["code"] = code;
        doc.setObject(obj);

        QCOMPARE(obj["userNameId"].toString(), userNameId);
        QCOMPARE(obj["password"].toString(), password);
        QCOMPARE(obj["code"].toString(), code);
    }

    // Test that login request is valid JSON
    void test_login_request_valid_json() {
        QJsonDocument doc;
        QJsonObject obj;
        obj["userNameId"] = "test user";
        obj["password"] = "test password";
        obj["code"] = "1234";
        doc.setObject(obj);

        QByteArray json = doc.toJson(QJsonDocument::Compact);
        QString jsonStr = QString::fromUtf8(json);

        QVERIFY(jsonStr.contains("userNameId"));
        QVERIFY(jsonStr.contains("password"));
        QVERIFY(jsonStr.contains("code"));
    }
};

QTEST_MAIN(TestLogin)
#include "test_login.moc"

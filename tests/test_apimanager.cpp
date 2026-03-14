#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QByteArray>
#include <QList>
#include <QPair>

#include "../src/apimanager.h"

class TestApiManager : public QObject {
    Q_OBJECT

private slots:
    // Test verification code response parsing
    void test_verification_code_response_parsing() {
        // Test valid integer response
        QByteArray response = "7156";
        bool ok = false;
        int code = response.toInt(&ok);

        QVERIFY(ok);
        QCOMPARE(code, 7156);
    }

    void test_verification_code_single_digit() {
        QByteArray response = "5";
        bool ok = false;
        int code = response.toInt(&ok);

        QVERIFY(ok);
        QCOMPARE(code, 5);
    }

    void test_verification_code_zero() {
        QByteArray response = "0";
        bool ok = false;
        int code = response.toInt(&ok);

        QVERIFY(ok);
        QCOMPARE(code, 0);
    }

    void test_verification_code_invalid_format() {
        QByteArray response = "invalid";
        bool ok = false;
        int code = response.toInt(&ok);

        QVERIFY(!ok);
        QCOMPARE(code, 0);
    }

    void test_verification_code_max_digits() {
        // Test with 6 digit code (maximum expected)
        QByteArray response = "999999";
        bool ok = false;
        int code = response.toInt(&ok);

        QVERIFY(ok);
        QCOMPARE(code, 999999);
    }

    // Test login response parsing - success case
    void test_login_success_response() {
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
        QVERIFY(token.contains("."));
    }

    // Test login failure response
    void test_login_failure_response() {
        QByteArray response = R"({"title":"操作提示","message":"用户名或密码错误","statusCode":401})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 401);
        QCOMPARE(obj["message"].toString(), QString("用户名或密码错误"));
    }

    void test_login_invalid_code_response() {
        QByteArray response = R"({"title":"操作提示","message":"验证码错误或已过期","statusCode":401})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 401);
    }

    // Test login with various status codes
    void test_login_server_error_response() {
        QByteArray response = R"({"title":"操作提示","message":"服务器内部错误","statusCode":500})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 500);
    }

    // Test login response edge cases
    void test_login_missing_status_code() {
        QByteArray response = R"({"title":"操作提示","message":"some message"})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 0);  // Default value
    }

    void test_login_missing_message() {
        QByteArray response = R"({"statusCode":200})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 200);
        QVERIFY(obj["message"].toString().isEmpty());
    }

    void test_login_empty_message() {
        QByteArray response = R"({"title":"操作提示","message":"","statusCode":200})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 200);
        QVERIFY(obj["message"].toString().isEmpty());
    }

    // Test invalid JSON handling
    void test_login_invalid_json() {
        QByteArray response = "not valid json";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error != QJsonParseError::NoError);
    }

    void test_login_malformed_json() {
        QByteArray response = R"({"statusCode":200, "message": "bearer;abc")";  // Missing closing brace

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error != QJsonParseError::NoError);
    }

    // Test bearer token extraction
    void test_bearer_token_extraction() {
        QString message = "bearer;eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIn0.dozjgNryP4J3jVmNHl0w5N_XgL0n3I9PlFUP0THsR8U";

        QVERIFY(message.startsWith("bearer;"));
        QString token = message.mid(7);
        QVERIFY(!token.isEmpty());
        QVERIFY(token.contains("."));
    }

    void test_bearer_token_empty() {
        QString message = "bearer;";

        QVERIFY(message.startsWith("bearer;"));
        QString token = message.mid(7);
        QVERIFY(token.isEmpty());
    }

    void test_bearer_token_no_prefix() {
        QString message = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIn0.dozjgNryP4J3jVmNHl0w5N_XgL0n3I9PlFUP0THsR8U";

        QVERIFY(!message.startsWith("bearer;"));
    }

    // Test日报 list response parsing
    void test_daily_report_list_response_parsing() {
        QByteArray response = R"({"total":1,"rows":[{"uuid":"A0A870CF87B2423EA44FEF4F333DFECC","creatorId":"SQ13793","creator":"孔启楠","createTime":"2026-03-12 12:59:27","dailyReportDate":"2026-03-12","month":"2026-03","week":"星期四","totalWorkingHours":8.5}],"pages":1})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["total"].toInt(), 1);
        QCOMPARE(obj["pages"].toInt(), 1);

        QJsonArray rows = obj["rows"].toArray();
        QCOMPARE(rows.size(), 1);

        QJsonObject firstRow = rows[0].toObject();
        QCOMPARE(firstRow["uuid"].toString(), QString("A0A870CF87B2423EA44FEF4F333DFECC"));
        QCOMPARE(firstRow["dailyReportDate"].toString(), QString("2026-03-12"));
        QCOMPARE(firstRow["month"].toString(), QString("2026-03"));
        QCOMPARE(firstRow["week"].toString(), QString("星期四"));
    }

    void test_daily_report_list_empty_response() {
        QByteArray response = R"({"total":0,"rows":[],"pages":0})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["total"].toInt(), 0);
        QCOMPARE(obj["rows"].toArray().size(), 0);
    }

    void test_daily_report_list_multiple_rows() {
        QByteArray response = R"({"total":2,"rows":[{"uuid":"uuid-1","dailyReportDate":"2026-03-12"},{"uuid":"uuid-2","dailyReportDate":"2026-03-13"}],"pages":1})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["total"].toInt(), 2);

        QJsonArray rows = obj["rows"].toArray();
        QCOMPARE(rows.size(), 2);

        QCOMPARE(rows[0].toObject()["uuid"].toString(), QString("uuid-1"));
        QCOMPARE(rows[1].toObject()["uuid"].toString(), QString("uuid-2"));
    }

    // Test日报 details response parsing
    void test_daily_report_details_response_parsing() {
        QByteArray response = R"([{"uuid":"task-1","taskDescription":"开组会","workingHours":4.0,"puuid":"report-uuid"},{"uuid":"task-2","taskDescription":"写文档","workingHours":4.5,"puuid":"report-uuid"}])";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonArray array = doc.array();
        QCOMPARE(array.size(), 2);

        QJsonObject firstTask = array[0].toObject();
        QCOMPARE(firstTask["taskDescription"].toString(), QString("开组会"));
        QCOMPARE(firstTask["workingHours"].toDouble(), 4.0);

        QJsonObject secondTask = array[1].toObject();
        QCOMPARE(secondTask["taskDescription"].toString(), QString("写文档"));
        QCOMPARE(secondTask["workingHours"].toDouble(), 4.5);
    }

    void test_daily_report_details_empty_array() {
        QByteArray response = "[]";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonArray array = doc.array();
        QCOMPARE(array.size(), 0);
    }

    // Test create/update日报 request building
    void test_create_daily_report_request_parameters() {
        QString applicantId = "SQ13793";
        QString applicantName = "孔启楠";
        QString dailyReportDate = "2026-03-13";
        QString month = "2026-03";
        QString week = "星期五";

        QList<QPair<QString, double>> tasks;
        tasks.append(qMakePair("开组会", 4.0));
        tasks.append(qMakePair("写文档", 4.5));

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

        QString queryStr = query.query(QUrl::FullyEncoded);
        QVERIFY(queryStr.contains("applicantId=SQ13793"));
        QVERIFY(queryStr.contains("applicantName="));
        QVERIFY(queryStr.contains("dailyReportDate=2026-03-13"));
        QVERIFY(queryStr.contains("month=2026-03"));
        QVERIFY(queryStr.contains("week="));
        QVERIFY(queryStr.contains("task1="));
        QVERIFY(queryStr.contains("hours1="));
        QVERIFY(queryStr.contains("task2="));
        QVERIFY(queryStr.contains("hours2="));
    }

    void test_update_daily_report_request_parameters() {
        QString uuid = "A0A870CF87B2423EA44FEF4F333DFECC";
        QString applicantId = "SQ13793";
        QString applicantName = "孔启楠";
        QString dailyReportDate = "2026-03-12";
        QString month = "2026-03";
        QString week = "星期四";

        QList<QPair<QString, double>> tasks;
        tasks.append(qMakePair("开组会，调研dify", 4.0));
        tasks.append(qMakePair("翻看业务相关文档", 4.5));

        QUrlQuery query;
        query.addQueryItem("uuid", uuid);
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

        QString queryStr = query.query(QUrl::FullyEncoded);
        QVERIFY(queryStr.contains("uuid=" + QUrl::toPercentEncoding(uuid)));
        QVERIFY(queryStr.contains("i=2"));
        QVERIFY(queryStr.contains("applicantId=SQ13793"));
    }

    // Test cookie header generation
    void test_cookie_header_generation() {
        QString token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIn0.dozjgNryP4J3jVmNHl0w5N_XgL0n3I9PlFUP0THsR8U";

        QString cookieHeader = "token=" + token;
        QVERIFY(cookieHeader.startsWith("token="));
        QVERIFY(cookieHeader.contains(token));
    }

    void test_cookie_header_empty_token() {
        QString token = "";
        QString cookieHeader = "token=" + token;
        QCOMPARE(cookieHeader, QString("token="));
    }

    // Test create日报 response - success
    void test_create_daily_report_success_response() {
        QByteArray response = R"({"title":"操作提示","message":"保存成功！","statusCode":200})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 200);
        QCOMPARE(obj["message"].toString(), QString("保存成功！"));
    }

    // Test create日报 response - already exists (statusCode 300)
    void test_create_daily_report_already_exists_response() {
        QByteArray response = R"({"title":"操作提示","message":"2026-03-14已存在日报，请勿重复添加！","statusCode":300})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 300);
        QString message = obj["message"].toString();
        QVERIFY(message.contains("已存在"));
    }

    // Test create日报 response - invalid JSON
    void test_create_daily_report_invalid_json_response() {
        QByteArray response = "not a json response";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error != QJsonParseError::NoError);
    }

    // Test sync日报 response - success
    void test_sync_daily_report_success_response() {
        QByteArray response = R"({"title":"操作提示","message":"更新成功！","statusCode":200})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 200);
        QCOMPARE(obj["message"].toString(), QString("更新成功！"));
    }

    // Test sync日报 response - not found
    void test_sync_daily_report_not_found_response() {
        QByteArray response = R"({"title":"操作提示","message":"日报不存在","statusCode":404})";

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        QVERIFY(parseError.error == QJsonParseError::NoError);

        QJsonObject obj = doc.object();
        QCOMPARE(obj["statusCode"].toInt(), 404);
    }

    // Test various title formats in responses
    void test_response_title_formats() {
        // Chinese title
        QByteArray response1 = R"({"title":"操作提示","message":"保存成功！","statusCode":200})";
        QJsonDocument doc1 = QJsonDocument::fromJson(response1);
        QCOMPARE(doc1.object()["title"].toString(), QString("操作提示"));

        // English title
        QByteArray response2 = R"({"title":"Success","message":"Saved","statusCode":200})";
        QJsonDocument doc2 = QJsonDocument::fromJson(response2);
        QCOMPARE(doc2.object()["title"].toString(), QString("Success"));
    }

    // Test date format in requests
    void test_date_format_validation() {
        QString date = "2026-03-13";

        // Should be in YYYY-MM-DD format
        QCOMPARE(date.length(), 10);
        QCOMPARE(date[4], QChar('-'));
        QCOMPARE(date[7], QChar('-'));
    }

    void test_month_format_validation() {
        QString month = "2026-03";

        // Should be in YYYY-MM format
        QCOMPARE(month.length(), 7);
        QCOMPARE(month[4], QChar('-'));
    }

    // Test week day validation
    void test_weekday_format_validation() {
        QStringList validWeekdays = {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"};

        QVERIFY(validWeekdays.contains("星期一"));
        QVERIFY(validWeekdays.contains("星期五"));
        QVERIFY(validWeekdays.contains("星期日"));
    }

    // Test task array building
    void test_task_array_with_single_task() {
        QList<QPair<QString, double>> tasks;
        tasks.append(qMakePair("单一任务", 2.5));

        QCOMPARE(tasks.size(), 1);
        QCOMPARE(tasks[0].first, QString("单一任务"));
        QCOMPARE(tasks[0].second, 2.5);
    }

    void test_task_array_with_multiple_tasks() {
        QList<QPair<QString, double>> tasks;
        tasks.append(qMakePair("任务1", 1.0));
        tasks.append(qMakePair("任务2", 2.0));
        tasks.append(qMakePair("任务3", 3.0));

        QCOMPARE(tasks.size(), 3);

        double totalHours = 0;
        for (const auto& task : tasks) {
            totalHours += task.second;
        }
        QCOMPARE(totalHours, 6.0);
    }

    void test_task_array_empty() {
        QList<QPair<QString, double>> tasks;

        QCOMPARE(tasks.size(), 0);
    }

    // Test request parameter encoding
    void test_chinese_parameter_encoding() {
        QString chineseText = "孔启楠";

        QString encoded = QUrl::toPercentEncoding(chineseText);
        // Chinese characters should be percent-encoded
        QVERIFY(encoded.contains('%'));
    }

    void test_chinese_task_description_encoding() {
        QString task = "开组会，调研dify";

        QString encoded = QUrl::toPercentEncoding(task);
        // Should contain percent encoding for some characters
        QVERIFY(!encoded.isEmpty());
    }
};

QTEST_MAIN(TestApiManager)
#include "test_apimanager.moc"

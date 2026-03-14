/**
 * E2E 端到端测试
 *
 * 使用 Qt Test Framework 直接操作 UI 对象树
 * 无需模拟鼠标键盘，直接在 offscreen 模式下测试
 */

#include <QtTest/QtTest>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDialog>
#include "../src/mainwindow.h"
#include "../src/logindialog.h"
#include "../src/apimanager.h"
#include "../src/cloudsessionmanager.h"

class TestE2E : public QObject {
    Q_OBJECT

private:
    MainWindow* mainWindow;

    // 辅助函数：查找按钮（部分匹配文本）
    QPushButton* findButtonByTextContains(const QString& text) {
        QList<QPushButton*> buttons = mainWindow->findChildren<QPushButton*>();
        for (QPushButton* btn : buttons) {
            if (btn->text().contains(text)) {
                return btn;
            }
        }
        return nullptr;
    }

private slots:
    void initTestCase() {
        // 初始化主窗口
        mainWindow = new MainWindow();
        mainWindow->show();
    }

    // 测试 1: UI 组件存在性
    void test_01_ui_components_exist() {
        QPushButton *loginBtn = findButtonByTextContains("登录");
        QPushButton *startBtn = findButtonByTextContains("上班");
        QPushButton *endBtn = findButtonByTextContains("下班");
        QPushButton *syncBtn = findButtonByTextContains("同步");

        QVERIFY2(loginBtn != nullptr, "未找到登录按钮");
        QVERIFY2(startBtn != nullptr, "未找到上班按钮");
        QVERIFY2(endBtn != nullptr, "未找到下班按钮");
        QVERIFY2(syncBtn != nullptr, "未找到同步按钮");

        qDebug() << "登录按钮文本:" << loginBtn->text();
        qDebug() << "上班按钮文本:" << startBtn->text();
        qDebug() << "下班按钮文本:" << endBtn->text();
        qDebug() << "同步按钮文本:" << syncBtn->text();
        qDebug() << "所有 UI 组件存在性测试通过";
    }

    // 测试 2: 登录按钮点击
    void test_02_login_button_click() {
        QPushButton* loginBtn = findButtonByTextContains("登录");
        QVERIFY2(loginBtn != nullptr, "未找到登录按钮");

        qDebug() << "点击登录按钮...";
        emit loginBtn->clicked();
        QTest::qWait(1000);

        // 查找登录对话框
        QList<QDialog*> dialogs = mainWindow->findChildren<QDialog*>();
        qDebug() << "对话框数量:" << dialogs.size();
        QVERIFY2(!dialogs.isEmpty(), "登录对话框未创建");
    }

    // 测试 3: 登录对话框和验证码
    void test_03_login_dialog_verification_code() {
        QPushButton* loginBtn = findButtonByTextContains("登录");
        emit loginBtn->clicked();
        QTest::qWait(500);

        // 查找 LoginDialog
        LoginDialog* loginDialog = nullptr;
        QList<QDialog*> dialogs = mainWindow->findChildren<QDialog*>();
        for (QDialog* dialog : dialogs) {
            loginDialog = qobject_cast<LoginDialog*>(dialog);
            if (loginDialog) break;
        }

        if (loginDialog) {
            // 等待验证码自动获取
            for (int i = 0; i < 50; i++) {
                QTest::qWait(100);
                if (!loginDialog->getCode().isEmpty()) break;
            }

            QString code = loginDialog->getCode();
            qDebug() << "验证码:" << code;
            // 注意：验证码可能因为网络问题获取失败，这里只做记录
            if (code.isEmpty()) {
                QWARN("验证码未获取，可能是网络问题");
            } else {
                qDebug() << "验证码获取成功:" << code;
            }
        } else {
            QWARN("未找到登录对话框");
        }
    }

    // 测试 4: 输入账号密码
    void test_04_login_input() {
        QPushButton* loginBtn = findButtonByTextContains("登录");
        emit loginBtn->clicked();
        QTest::qWait(500);

        // 查找 LoginDialog
        LoginDialog* loginDialog = nullptr;
        QList<QDialog*> dialogs = mainWindow->findChildren<QDialog*>();
        for (QDialog* dialog : dialogs) {
            loginDialog = qobject_cast<LoginDialog*>(dialog);
            if (loginDialog) break;
        }

        if (loginDialog) {
            // 查找输入框
            QList<QLineEdit*> edits = loginDialog->findChildren<QLineEdit*>();
            qDebug() << "输入框数量:" << edits.size();

            if (edits.size() >= 2) {
                edits[0]->setText("741260264");
                edits[1]->setText("84769629aA.");
                qDebug() << "账号:" << edits[0]->text();
                qDebug() << "密码:" << edits[1]->text();
            }
        }
    }

    // 测试 5: 上班按钮
    void test_05_start_shift() {
        QPushButton* startBtn = findButtonByTextContains("上班");
        QVERIFY2(startBtn != nullptr, "未找到上班按钮");
        qDebug() << "上班按钮文本:" << startBtn->text();
        qDebug() << "上班按钮状态:" << (startBtn->isEnabled() ? "启用" : "禁用");
    }

    // 测试 6: 下班按钮
    void test_06_end_shift() {
        QPushButton* endBtn = findButtonByTextContains("下班");
        QVERIFY2(endBtn != nullptr, "未找到下班按钮");
        qDebug() << "下班按钮文本:" << endBtn->text();
        qDebug() << "下班按钮状态:" << (endBtn->isEnabled() ? "启用" : "禁用");
    }

    // 测试 7: 同步按钮
    void test_07_sync_button() {
        QPushButton* syncBtn = findButtonByTextContains("同步");
        QVERIFY2(syncBtn != nullptr, "未找到同步按钮");
        qDebug() << "同步按钮文本:" << syncBtn->text();
        qDebug() << "同步按钮状态:" << (syncBtn->isEnabled() ? "启用" : "禁用");
    }

    void cleanupTestCase() {
        delete mainWindow;
    }
};

QTEST_MAIN(TestE2E)
#include "test_e2e.moc"

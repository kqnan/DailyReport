#include <QTest>
#include <QFile>
#include "../src/logindialog.h"
#include "../src/utils.h"

class TestRememberPassword : public QObject {
    Q_OBJECT

private slots:
    void init() {
        // Clean up before each test
        QString credPath = getStorageDirectory() + "/credentials";
        QFile::remove(credPath);
    }

    void cleanupTestCase() {
        // Clean up test credentials file
        QString credPath = getStorageDirectory() + "/credentials";
        QFile::remove(credPath);
    }

    void testEncryptDecrypt() {
        LoginDialog dialog;
        QString testPassword = "mySecretPassword123";

        // Save credentials
        dialog.saveCredentials("testuser", testPassword);

        // Verify file was created
        QString credPath = getStorageDirectory() + "/credentials";
        QVERIFY(QFile::exists(credPath));

        // Create a new dialog and load credentials
        LoginDialog dialog2;
        dialog2.loadSavedCredentials();

        // Verify loaded values (getters read from UI fields)
        QCOMPARE(dialog2.getUserNameId(), "testuser");
        QCOMPARE(dialog2.getPassword(), testPassword);
    }

    void testEmptyPassword() {
        LoginDialog dialog;

        // Test with empty password - should not crash
        dialog.saveCredentials("testuser", "");

        // File should not contain the username with empty password
        QString credPath = getStorageDirectory() + "/credentials";
        if (QFile::exists(credPath)) {
            QFile file(credPath);
            QVERIFY(file.open(QIODevice::ReadOnly));
            QString content = QString::fromUtf8(file.readAll());
            // Should not contain this user
            QVERIFY(!content.contains("testuser|"));
        }
    }

    void testCorruptedFile() {
        // Create a corrupted credentials file
        QString credPath = getStorageDirectory() + "/credentials";
        QFile file(credPath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("invalid_format_no_pipe");
        file.close();

        // Try to load - should handle gracefully
        LoginDialog dialog;
        dialog.loadSavedCredentials();

        // File should be removed due to invalid format
        QVERIFY(!QFile::exists(credPath));
    }

    void testClearCredentials() {
        LoginDialog dialog;

        // Save credentials first
        dialog.saveCredentials("testuser", "testpass");
        QString credPath = getStorageDirectory() + "/credentials";
        QVERIFY(QFile::exists(credPath));

        // Clear credentials
        dialog.clearSavedCredentials();

        // File should be removed
        QVERIFY(!QFile::exists(credPath));
    }
};

QTEST_MAIN(TestRememberPassword)
#include "test_remember_password.moc"

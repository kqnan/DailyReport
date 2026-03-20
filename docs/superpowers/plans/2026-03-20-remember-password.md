# 记住密码功能实现计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在登录对话框添加"记住密码"功能，支持加密存储和自动填充用户凭证。

**Architecture:** 在 LoginDialog 类中添加复选框和凭证管理功能，使用 XOR+Base64 简单加密，存储到应用数据目录。

**Tech Stack:** Qt 6 (QCheckBox, QFile, QByteArray), C++20

---

## 文件结构

**修改文件:**
- `src/logindialog.h`: 添加复选框成员和凭证管理函数声明
- `src/logindialog.cpp`: 实现加密/解密、文件读写、UI 初始化和保存逻辑

**测试文件:**
- `tests/test_remember_password.cpp`: 新建单元测试文件，测试加密解密功能

---

## Chunk 1: 添加复选框和函数声明

### Task 1: 修改 logindialog.h 添加成员和函数声明

**Files:**
- Modify: `src/logindialog.h`

- [ ] **Step 1: 添加 QCheckBox 头文件和成员变量**

在 `src/logindialog.h` 中：
1. 在现有 includes 后添加 `#include <QCheckBox>`
2. 在 private 部分添加 `QCheckBox *rememberCheckBox;`
3. 添加私有函数声明：

```cpp
private:
    void loadSavedCredentials();
    void saveCredentials(const QString& username, const QString& password);
    void clearSavedCredentials();
    QString encryptPassword(const QString& password);
    QString decryptPassword(const QString& encrypted);
```

- [ ] **Step 2: 构建并验证**

Run: `cd /root/DailyReport/build && cmake --build .`
Expected: Build succeeds with no errors

- [ ] **Step 3: Commit**

```bash
git add src/logindialog.h
git commit -m "feat: add remember password checkbox and function declarations"
```

---

## Chunk 2: 实现加密解密和文件操作

### Task 2: 实现加密解密函数

**Files:**
- Modify: `src/logindialog.cpp`

- [ ] **Step 1: 添加 encryptPassword 函数**

在 `src/logindialog.cpp` 末尾添加（在类外部或作为成员函数）：

```cpp
QString LoginDialog::encryptPassword(const QString& password) {
    if (password.isEmpty()) return QString();

    QString key = "DailyReport2026";
    QByteArray result;
    QByteArray pwdBytes = password.toUtf8();

    for (int i = 0; i < pwdBytes.size(); ++i) {
        result.append(pwdBytes[i] ^ key[i % key.length()].toLatin1());
    }
    return result.toBase64();
}

QString LoginDialog::decryptPassword(const QString& encrypted) {
    if (encrypted.isEmpty()) return QString();

    QString key = "DailyReport2026";
    QByteArray encBytes = QByteArray::fromBase64(encrypted.toUtf8());
    if (encBytes.isEmpty()) return QString();

    QByteArray result;
    for (int i = 0; i < encBytes.size(); ++i) {
        result.append(encBytes[i] ^ key[i % key.length()].toLatin1());
    }
    return QString::fromUtf8(result);
}
```

- [ ] **Step 2: 添加必要的 includes**

在 `src/logindialog.cpp` 顶部添加：
```cpp
#include <QFile>
#include <QTextStream>
#include "utils.h"
```

- [ ] **Step 3: 构建并验证**

Run: `cd /root/DailyReport/build && cmake --build .`
Expected: Build succeeds with no errors

- [ ] **Step 4: Commit**

```bash
git add src/logindialog.cpp
git commit -m "feat: implement password encryption and decryption"
```

---

### Task 3: 实现凭证加载和保存函数

**Files:**
- Modify: `src/logindialog.cpp`

- [ ] **Step 1: 实现 loadSavedCredentials 函数**

在 `src/logindialog.cpp` 添加：

```cpp
void LoginDialog::loadSavedCredentials() {
    QString credPath = getStorageDirectory() + "/credentials";
    QFile file(credPath);

    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        rememberCheckBox->setChecked(false);
        return;
    }

    QTextStream in(&file);
    QString content = in.readLine().trimmed();
    file.close();

    if (content.isEmpty()) {
        rememberCheckBox->setChecked(false);
        return;
    }

    // Parse format: username|encrypted_password
    QStringList parts = content.split("|");
    if (parts.size() != 2) {
        qDebug() << "Invalid credentials format";
        rememberCheckBox->setChecked(false);
        file.remove(); // Remove corrupted file
        return;
    }

    QString username = parts[0];
    QString encryptedPwd = parts[1];
    QString password = decryptPassword(encryptedPwd);

    if (password.isEmpty()) {
        qDebug() << "Failed to decrypt password";
        rememberCheckBox->setChecked(false);
        file.remove(); // Remove corrupted file
        return;
    }

    // Fill in the credentials
    userNameIdEdit->setText(username);
    passwordEdit->setText(password);
    rememberCheckBox->setChecked(true);
}
```

- [ ] **Step 2: 实现 saveCredentials 函数**

```cpp
void LoginDialog::saveCredentials(const QString& username, const QString& password) {
    if (username.isEmpty() || password.isEmpty()) return;

    QString encryptedPwd = encryptPassword(password);
    if (encryptedPwd.isEmpty()) {
        qDebug() << "Failed to encrypt password";
        return;
    }

    QString credPath = getStorageDirectory() + "/credentials";
    QFile file(credPath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open credentials file for writing:" << file.errorString();
        return;
    }

    QTextStream out(&file);
    out << username << "|" << encryptedPwd;
    file.close();

    qDebug() << "Credentials saved successfully";
}
```

- [ ] **Step 3: 实现 clearSavedCredentials 函数**

```cpp
void LoginDialog::clearSavedCredentials() {
    QString credPath = getStorageDirectory() + "/credentials";
    QFile file(credPath);

    if (file.exists()) {
        if (file.remove()) {
            qDebug() << "Credentials file removed";
        } else {
            qDebug() << "Failed to remove credentials file:" << file.errorString();
        }
    }
}
```

- [ ] **Step 4: 构建并验证**

Run: `cd /root/DailyReport/build && cmake --build .`
Expected: Build succeeds with no errors

- [ ] **Step 5: Commit**

```bash
git add src/logindialog.cpp
git commit -m "feat: implement credential load and save functions"
```

---

## Chunk 3: UI 集成和登录逻辑

### Task 4: 添加复选框 UI 和初始化

**Files:**
- Modify: `src/logindialog.cpp` (构造函数)

- [ ] **Step 1: 在构造函数中创建复选框**

在 `src/logindialog.cpp` 的构造函数中，找到 `buttonLayout` 部分，在 `setLayout(layout)` 之前添加：

```cpp
    // Add remember password checkbox
    rememberCheckBox = new QCheckBox("记住密码");
    layout->addWidget(rememberCheckBox);
```

具体位置：在以下代码之后添加：
```cpp
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(getCodeButton);
    buttonLayout->addWidget(loginButton);
    layout->addLayout(buttonLayout);

    // Add remember password checkbox (HERE)
    rememberCheckBox = new QCheckBox("记住密码");
    layout->addWidget(rememberCheckBox);

    layout->addWidget(statusLabel);
```

- [ ] **Step 2: 在构造函数中调用 loadSavedCredentials**

在构造函数末尾（`onGetVerificationCode()` 之后）添加：

```cpp
    // Load saved credentials if any
    loadSavedCredentials();
```

- [ ] **Step 3: 构建并验证**

Run: `cd /root/DailyReport/build && cmake --build .`
Expected: Build succeeds with no errors

- [ ] **Step 4: Commit**

```bash
git add src/logindialog.cpp
git commit -m "feat: add remember password checkbox UI and initialization"
```

---

### Task 5: 修改登录成功逻辑

**Files:**
- Modify: `src/logindialog.cpp` (onLoginSuccess 函数)

- [ ] **Step 1: 修改 onLoginSuccess 函数**

将现有的 `onLoginSuccess` 函数替换为：

```cpp
void LoginDialog::onLoginSuccess(const QString& token) {
    statusLabel->setText("登录成功！");
    statusLabel->setStyleSheet("color: green;");

    // Save or clear credentials based on checkbox state
    if (rememberCheckBox->isChecked()) {
        saveCredentials(userNameIdEdit->text(), passwordEdit->text());
    } else {
        clearSavedCredentials();
    }

    emit loginCompleted(token);
    accept();
}
```

- [ ] **Step 2: 构建并验证**

Run: `cd /root/DailyReport/build && cmake --build .`
Expected: Build succeeds with no errors

- [ ] **Step 3: Commit**

```bash
git add src/logindialog.cpp
git commit -m "feat: integrate credential save logic on successful login"
```

---

## Chunk 4: 添加单元测试

### Task 6: 创建加密解密单元测试

**Files:**
- Create: `tests/test_remember_password.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 创建测试文件**

创建 `tests/test_remember_password.cpp`：

```cpp
#include <QtTest/QtTest>
#include <QFile>
#include "../src/logindialog.h"
#include "../src/utils.h"

class TestRememberPassword : public QObject {
    Q_OBJECT

private slots:
    void cleanupTestCase() {
        // Clean up test credentials file
        QString credPath = getStorageDirectory() + "/credentials";
        QFile::remove(credPath);
    }

    void testEncryptDecrypt() {
        // Create a temporary LoginDialog to test private methods
        // We need to make the methods accessible for testing
        LoginDialog dialog;

        QString testPassword = "mySecretPassword123";

        // Access private methods through public interface for testing
        // We'll test by using save/load functionality
        dialog.saveCredentials("testuser", testPassword);

        // Verify file was created
        QString credPath = getStorageDirectory() + "/credentials";
        QVERIFY(QFile::exists(credPath));

        // Now create a new dialog and load credentials
        LoginDialog dialog2;
        dialog2.loadSavedCredentials();

        // Verify loaded values (need to use public getters)
        QCOMPARE(dialog2.getUserNameId(), "testuser");
        QCOMPARE(dialog2.getPassword(), testPassword);
    }

    void testEmptyPassword() {
        LoginDialog dialog;

        // Test with empty password - should not crash
        dialog.saveCredentials("testuser", "");

        // File should not be created or be empty
        QString credPath = getStorageDirectory() + "/credentials";
        if (QFile::exists(credPath)) {
            QFile file(credPath);
            QVERIFY(file.open(QIODevice::ReadOnly));
            QString content = QString::fromUtf8(file.readAll());
            // Should not contain empty password entry
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
};

QTEST_MAIN(TestRememberPassword)
#include "test_remember_password.moc"
```

- [ ] **Step 2: 修改 logindialog.h 支持测试**

为了让测试可以访问私有方法，需要将方法改为 protected 或添加测试友元。更简单的方案是：将加密解密函数改为 `public` 或 `protected`。

修改 `src/logindialog.h`：
```cpp
// Change these from private to protected so tests can access them
protected:
    void loadSavedCredentials();
    void saveCredentials(const QString& username, const QString& password);
    void clearSavedCredentials();
    QString encryptPassword(const QString& password);
    QString decryptPassword(const QString& encrypted);
```

- [ ] **Step 3: 修改 CMakeLists.txt 添加测试**

在 `CMakeLists.txt` 中找到其他测试的添加位置，添加新的测试：

```cmake
add_executable(test_remember_password
    tests/test_remember_password.cpp
    src/logindialog.h
    src/logindialog.cpp
    src/apimanager.h
    src/apimanager.cpp
    src/utils.h
    src/utils.cpp
)

target_link_libraries(test_remember_password PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Network
    Qt6::Test
)
```

- [ ] **Step 4: 构建测试**

Run: `cd /root/DailyReport/build && cmake .. && cmake --build .`
Expected: Build succeeds, test_remember_password executable created

- [ ] **Step 5: 运行测试**

Run: `QT_QPA_PLATFORM=offscreen ./test_remember_password`
Expected: All tests pass

- [ ] **Step 6: Commit**

```bash
git add tests/test_remember_password.cpp CMakeLists.txt src/logindialog.h
git commit -m "test: add unit tests for remember password feature"
```

---

## 验证清单

- [ ] 登录对话框显示"记住密码"复选框
- [ ] 勾选复选框后登录，credentials 文件创建在应用数据目录
- [ ] 文件内容为 `username|encrypted_password` 格式
- [ ] 下次打开登录框，自动填充账号密码，复选框勾选
- [ ] 取消勾选后登录，credentials 文件被删除
- [ ] 加密后的密码无法直接看出原始密码
- [ ] 单元测试编译通过，所有测试通过

---

## 提交历史

预期提交顺序:
1. `feat: add remember password checkbox and function declarations`
2. `feat: implement password encryption and decryption`
3. `feat: implement credential load and save functions`
4. `feat: add remember password checkbox UI and initialization`
5. `feat: integrate credential save logic on successful login`
6. `test: add unit tests for remember password feature`

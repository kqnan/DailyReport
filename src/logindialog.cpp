#include "logindialog.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include "utils.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , apiManager(&ApiManager::instance())
{
    setWindowTitle("登录");
    setMinimumSize(300, 200);

    // Create widgets
    userNameIdEdit = new QLineEdit();
    userNameIdEdit->setPlaceholderText("账号（工号）");
    userNameIdEdit->setClearButtonEnabled(true);

    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setClearButtonEnabled(true);

    codeEdit = new QLineEdit();
    codeEdit->setPlaceholderText("验证码");
    codeEdit->setClearButtonEnabled(true);

    getCodeButton = new QPushButton("获取验证码");
    loginButton = new QPushButton("登录");
    loginButton->setDefault(true);

    statusLabel = new QLabel("就绪");
    statusLabel->setStyleSheet("color: gray;");

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("账号:"));
    layout->addWidget(userNameIdEdit);
    layout->addWidget(new QLabel("密码:"));
    layout->addWidget(passwordEdit);
    layout->addWidget(new QLabel("验证码:"));
    layout->addWidget(codeEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(getCodeButton);
    buttonLayout->addWidget(loginButton);
    layout->addLayout(buttonLayout);

    // Add remember password checkbox
    rememberCheckBox = new QCheckBox("记住密码");
    layout->addWidget(rememberCheckBox);

    layout->addWidget(statusLabel);

    setLayout(layout);

    // Auto-get verification code when dialog opens
    onGetVerificationCode();

    // Connect signals
    connect(getCodeButton, &QPushButton::clicked, this, &LoginDialog::onGetVerificationCode);
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(apiManager, &ApiManager::verificationCodeReceived, this, &LoginDialog::onVerificationCodeReceived);
    connect(apiManager, &ApiManager::verificationCodeFailed, this, [this](const QString& msg) {
        statusLabel->setText("获取验证码失败: " + msg);
        statusLabel->setStyleSheet("color: red;");
    });
    connect(apiManager, &ApiManager::loginSuccess, this, &LoginDialog::onLoginSuccess);
    connect(apiManager, &ApiManager::loginFailed, this, &LoginDialog::onLoginFailed);

    // Load saved credentials if any
    loadSavedCredentials();
}

LoginDialog::~LoginDialog() = default;

QString LoginDialog::getUserNameId() const {
    return userNameIdEdit->text();
}

QString LoginDialog::getPassword() const {
    return passwordEdit->text();
}

QString LoginDialog::getCode() const {
    return codeEdit->text();
}

void LoginDialog::onGetVerificationCode() {
    statusLabel->setText("正在获取验证码...");
    statusLabel->setStyleSheet("color: blue;");
    apiManager->getVerificationCode();
}

void LoginDialog::onVerificationCodeReceived(int code) {
    statusLabel->setText(QString("验证码: %1").arg(code));
    statusLabel->setStyleSheet("color: green;");
    codeEdit->setText(QString::number(code));
}

void LoginDialog::onLoginClicked() {
    if (userNameIdEdit->text().isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入账号");
        return;
    }
    if (passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入密码");
        return;
    }
    if (codeEdit->text().isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入验证码");
        return;
    }

    loginButton->setEnabled(false);
    statusLabel->setText("正在登录...");
    statusLabel->setStyleSheet("color: blue;");

    apiManager->login(getUserNameId(), getPassword(), getCode());
}

void LoginDialog::onLoginSuccess(const QString& token) {
    statusLabel->setText("登录成功！");
    statusLabel->setStyleSheet("color: green;");
    emit loginCompleted(token);  // Emit signal for parent window
    accept();  // Close dialog with Accepted result
}

void LoginDialog::onLoginFailed(const QString& message) {
    statusLabel->setText("登录失败: " + message);
    statusLabel->setStyleSheet("color: red;");
    loginButton->setEnabled(true);
}

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

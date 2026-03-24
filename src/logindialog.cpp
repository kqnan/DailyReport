#include "logindialog.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include "utils.h"
#include "animationutils.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , apiManager(&ApiManager::instance())
{
    setWindowTitle("登录");
    setMinimumSize(300, 200);

    // Apply Morandi color palette QSS styles
    QString morandiStyle = R"(
        QDialog {
            background-color: #F0EDE8;
        }
        QLabel {
            color: #4A4A4A;
            font-size: 14px;
        }
        QLineEdit {
            background-color: #FAF8F5;
            border: 1px solid #D4CFC7;
            border-radius: 6px;
            padding: 8px 12px;
            color: #4A4A4A;
            font-size: 13px;
        }
        QLineEdit:focus {
            border: 1px solid #6B8E6B;
        }
        QCheckBox {
            color: #4A4A4A;
            font-size: 13px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border-radius: 3px;
            border: 1px solid #D4CFC7;
            background-color: #FAF8F5;
        }
        QCheckBox::indicator:checked {
            background-color: #6B8E6B;
            border-color: #6B8E6B;
        }
    )";
    setStyleSheet(morandiStyle);

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

    // Create RippleButton for "获取验证码" with Secondary color (雾蓝)
    getCodeButton = new RippleButton("获取验证码");
    getCodeButton->setStyleSheet(R"(
        RippleButton {
            background-color: #6B8BA3;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 13px;
        }
        RippleButton:hover {
            background-color: #5A7A93;
        }
        RippleButton:pressed {
            background-color: #4A6A83;
        }
    )");
    getCodeButton->setRippleColor(QColor(255, 255, 255, 60));

    // Create RippleButton for "登录" with Primary color (鼠尾草绿)
    loginButton = new RippleButton("登录");
    loginButton->setDefault(true);
    loginButton->setStyleSheet(R"(
        RippleButton {
            background-color: #6B8E6B;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: bold;
        }
        RippleButton:hover {
            background-color: #5D7D5D;
        }
        RippleButton:pressed {
            background-color: #4F6D4F;
        }
    )");
    loginButton->setRippleColor(QColor(255, 255, 255, 80));

    statusLabel = new QLabel("就绪");
    statusLabel->setStyleSheet("color: #8B8680; font-size: 12px;");

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->addWidget(new QLabel("账号:"));
    layout->addWidget(userNameIdEdit);
    layout->addWidget(new QLabel("密码:"));
    layout->addWidget(passwordEdit);
    layout->addWidget(new QLabel("验证码:"));
    layout->addWidget(codeEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
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
        statusLabel->setStyleSheet("color: #B87A7A; font-size: 12px;");
    });
    connect(apiManager, &ApiManager::loginSuccess, this, &LoginDialog::onLoginSuccess);
    connect(apiManager, &ApiManager::loginFailed, this, &LoginDialog::onLoginFailed);

    // Load saved credentials if any
    loadSavedCredentials();

    // Window fade-in animation
    if (AnimationUtils::animationsEnabled()) {
        setWindowOpacity(0.0);
        QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "windowOpacity");
        fadeIn->setDuration(300);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::InOutQuad);
        fadeIn->start(QPropertyAnimation::DeleteWhenStopped);
    }
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
    statusLabel->setStyleSheet("color: #6B8BA3; font-size: 12px;");
    apiManager->getVerificationCode();
}

void LoginDialog::onVerificationCodeReceived(int code) {
    statusLabel->setText(QString("验证码: %1").arg(code));
    statusLabel->setStyleSheet("color: #6B8E6B; font-size: 12px;");
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
    statusLabel->setStyleSheet("color: #6B8BA3; font-size: 12px;");

    apiManager->login(getUserNameId(), getPassword(), getCode());
}

void LoginDialog::onLoginSuccess(const QString& token) {
    statusLabel->setText("登录成功！");
    statusLabel->setStyleSheet("color: #6B8E6B; font-size: 12px;");

    // Save or clear credentials based on checkbox state
    if (rememberCheckBox->isChecked()) {
        saveCredentials(userNameIdEdit->text(), passwordEdit->text());
    } else {
        clearSavedCredentials();
    }

    emit loginCompleted(token);
    accept();
}

void LoginDialog::onLoginFailed(const QString& message) {
    statusLabel->setText("登录失败: " + message);
    statusLabel->setStyleSheet("color: #B87A7A; font-size: 12px;");
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

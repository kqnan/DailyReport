#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include "apimanager.h"

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    QString getUserNameId() const;
    QString getPassword() const;
    QString getCode() const;

    // Credential persistence functions
    void loadSavedCredentials();
    void saveCredentials(const QString& username, const QString& password);
    void clearSavedCredentials();
    QString encryptPassword(const QString& password);
    QString decryptPassword(const QString& encrypted);

signals:
    void loginCompleted(const QString& token);

private slots:
    void onGetVerificationCode();
    void onVerificationCodeReceived(int code);
    void onLoginClicked();
    void onLoginSuccess(const QString& token);
    void onLoginFailed(const QString& message);

private:
    QLineEdit *userNameIdEdit;
    QLineEdit *passwordEdit;
    QLineEdit *codeEdit;
    QPushButton *getCodeButton;
    QPushButton *loginButton;
    QLabel *statusLabel;
    QCheckBox *rememberCheckBox;

    ApiManager* apiManager;
};

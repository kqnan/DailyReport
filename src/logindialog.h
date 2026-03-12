#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "apimanager.h"

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    QString getUserNameId() const;
    QString getPassword() const;
    QString getCode() const;

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

    ApiManager* apiManager;
};

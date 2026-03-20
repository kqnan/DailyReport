# 记住密码功能设计文档

## 功能描述

在登录对话框添加"记住密码"功能。用户勾选复选框后登录成功，账号和加密后的密码保存到应用数据目录。下次打开登录框时自动填充账号和密码。

## 需求详情

### 1. UI 设计

```
账号: [_______________]
密码: [_______________]
验证码: [_______] [获取验证码]
[✓] 记住密码          ← 新增 QCheckBox
      [登录]
```

### 2. 加密方式

使用简单混淆（XOR + Base64）：
- 固定密钥："DailyReport2026"
- 加密流程：密码字符串 → 与密钥循环异或 → Base64 编码
- 解密流程：Base64 解码 → 与密钥循环异或 → 原始密码

### 3. 存储格式

- **文件路径**：`~/.DailyReport/credentials`
- **格式**：纯文本，内容为 `username|encrypted_password`
- 示例：`SQ13793|QmFzZTY0RW5jcnlwdGVkU3RyaW5n`

### 4. 交互流程

**打开登录对话框时：**
1. 检查 `~/.DailyReport/credentials` 是否存在
2. 若存在，读取并解密账号密码
3. 填充到输入框，勾选"记住密码"复选框

**点击登录按钮时：**
1. 验证账号密码验证码
2. 登录成功
3. 若"记住密码"已勾选 → 加密保存账号密码到文件
4. 若"记住密码"未勾选 → 删除 credentials 文件（如果存在）

## 技术实现

### 修改文件

**src/logindialog.h**
- 添加 `QCheckBox *rememberCheckBox` 成员
- 添加 `void loadSavedCredentials()` 函数
- 添加 `void saveCredentials(const QString& username, const QString& password)` 函数
- 添加 `QString encryptPassword(const QString& password)` 函数
- 添加 `QString decryptPassword(const QString& encrypted)` 函数

**src/logindialog.cpp**
- 构造函数：创建复选框，调用 `loadSavedCredentials()`
- 实现加密/解密函数（XOR + Base64）
- 实现文件读写函数
- `onLoginSuccess`：根据复选框状态保存或清除凭证

### 关键代码片段

```cpp
// 加密密码
QString LoginDialog::encryptPassword(const QString& password) {
    QString key = "DailyReport2026";
    QByteArray result;
    QByteArray pwdBytes = password.toUtf8();
    for (int i = 0; i < pwdBytes.size(); ++i) {
        result.append(pwdBytes[i] ^ key[i % key.length()].toLatin1());
    }
    return result.toBase64();
}

// 解密密码
QString LoginDialog::decryptPassword(const QString& encrypted) {
    QString key = "DailyReport2026";
    QByteArray encBytes = QByteArray::fromBase64(encrypted.toUtf8());
    QByteArray result;
    for (int i = 0; i < encBytes.size(); ++i) {
        result.append(encBytes[i] ^ key[i % key.length()].toLatin1());
    }
    return QString::fromUtf8(result);
}
```

## 验收标准

- [ ] 登录对话框显示"记住密码"复选框
- [ ] 勾选后登录成功，账号密码加密保存到 `~/.DailyReport/credentials`
- [ ] 下次打开登录框，自动填充保存的账号密码
- [ ] 取消勾选后登录，删除已保存的凭证文件
- [ ] 加密后的密码无法直接看出原始密码

## 安全说明

此方案使用简单混淆，不是高强度加密。主要目的：
1. 防止密码以明文形式存储在磁盘上
2. 防止旁观者直接看到密码

不适用于高安全要求的场景。

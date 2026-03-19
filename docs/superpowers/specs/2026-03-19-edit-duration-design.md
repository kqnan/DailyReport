# 编辑工作时长功能设计文档

## 功能描述

在双击工作记录弹出的编辑对话框中，添加工作时长编辑功能。用户可以修改已结束工作记录的时长，系统会自动重新计算结束时间。

## 需求详情

### 1. 阻止未结束会话编辑
- 当用户双击一个正在进行中的工作记录（endTime 为空）时，不弹出编辑对话框
- 直接返回，不做任何操作

### 2. 添加时长编辑框
- 在编辑对话框中添加一个文本输入框（QLineEdit）
- 使用 QDoubleValidator 限制输入：正数，最多1位小数
- 默认值为当前工作记录的 workingHours（从 buffer 或 session 中获取）

### 3. 计算新的结束时间
- 保存时，根据新的开始时间和编辑后的时长计算新的 endTime
- 计算公式：newEndTime = startTime + (durationHours * 3600 seconds)
- 使用 QDateTime::addSecs() 进行时间计算

### 4. 更新 buffer
- 调用 CloudSessionManager::updateSession() 更新 session
- 同时更新 durationHours 和 endTime 字段
- Buffer 会自动同步 workingHours 字段

## 界面设计

```
编辑记录
─────────────────────
开始时间: 2026-03-19 09:00
结束时间: 2026-03-19 10:30
状态: 已结束

工作内容:
[工作内容编辑框]

工作类型:
[工作类型下拉框]

工作时长(小时):
[1.5]          ← 新增

[取消]  [保存]
```

## 数据流

```
用户双击已结束记录
    ↓
检查 endTime 是否为空 → 为空则直接返回
    ↓
显示对话框（含时长编辑框，默认值为当前 durationHours）
    ↓
用户编辑时长并保存
    ↓
计算: newEndTime = startTime + (durationHours * 3600)
    ↓
更新 session: durationHours, endTime, activity, workType
    ↓
调用 CloudSessionManager::updateSession() 同步到 buffer
    ↓
刷新界面显示
```

## 技术实现

### 修改文件
- `src/mainwindow.cpp`: 修改 `onEditSession()` 函数

### 关键代码片段

```cpp
// 1. 阻止未结束会话
if (session->endTime.isEmpty()) {
    return; // 未结束的会话不可编辑
}

// 2. 添加时长输入框
QLineEdit *durationEdit = new QLineEdit();
durationEdit->setText(QString::number(session->durationHours, 'f', 1));
QDoubleValidator *validator = new QDoubleValidator(0.1, 24.0, 1, durationEdit);
durationEdit->setValidator(validator);
layout->addWidget(new QLabel("工作时长(小时):"));
layout->addWidget(durationEdit);

// 3. 保存时计算新的结束时间
if (dialog->exec() == QDialog::Accepted) {
    double newDuration = durationEdit->text().toDouble();
    QDateTime startDt = QDateTime::fromString(session->startTime, Qt::ISODate);
    QDateTime newEndDt = startDt.addSecs(qRound(newDuration * 3600));

    session->durationHours = newDuration;
    session->endTime = newEndDt.toString(Qt::ISODate);
    session->activity = activityEdit->toPlainText();
    session->workType = workTypeCombo->currentText();

    CloudSessionManager::instance().updateSession(*session);
    loadSessions(session->date);
}
```

## 单元测试

需要添加单元测试验证：
1. 未结束会话（endTime为空）双击时不打开对话框
2. 编辑时长后，durationHours 正确更新
3. 编辑时长后，endTime 根据 startTime + duration 正确计算
4. 编辑后 buffer 中的数据正确更新

## 验收标准

- [ ] 双击未结束记录时，不弹出编辑对话框
- [ ] 双击已结束记录时，弹出编辑对话框并显示当前工作时长
- [ ] 工作时长输入框只能输入正数，最多1位小数
- [ ] 保存后，buffer 中的 workingHours 和 endTime 正确更新
- [ ] 新的 endTime = startTime + (durationHours * 3600 seconds)
- [ ] 单元测试通过

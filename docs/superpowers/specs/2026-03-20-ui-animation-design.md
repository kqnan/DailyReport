# UI 动画与美化设计文档

## 功能描述

为工时记录应用添加现代化 UI 动效，提升用户体验和视觉美感。

## 需求详情

### 1. 圆角边框

使用 QSS (Qt Style Sheets) 为所有 UI 元素添加圆角：

| 元素 | 圆角半径 | 说明 |
|------|----------|------|
| 主窗口 | `border-radius: 16px` | 整体窗口圆角 |
| 按钮 | `border-radius: 12px` | 已有基础上统一 |
| 输入框 | `border-radius: 8px` | 账号、密码输入框 |
| 列表项 | `border-radius: 8px` | 工作片段列表项 |
| 对话框 | `border-radius: 12px` | 登录对话框等 |

### 2. 按钮涟漪效果

创建自定义 `RippleButton` 类继承 `QPushButton`：

**公共接口：**
```cpp
class RippleButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QColor rippleColor READ rippleColor WRITE setRippleColor)
    Q_PROPERTY(int rippleDuration READ rippleDuration WRITE setRippleDuration)

public:
    explicit RippleButton(QWidget *parent = nullptr);
    explicit RippleButton(const QString &text, QWidget *parent = nullptr);

    void setRippleColor(const QColor &color);
    QColor rippleColor() const;

    void setRippleDuration(int msecs);
    int rippleDuration() const;

    void setRippleEnabled(bool enabled);
    bool isRippleEnabled() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_rippleColor = QColor(255, 255, 255, 100); // 默认白色半透明
    int m_rippleDuration = 400; // 默认400ms
    bool m_rippleEnabled = true;
    // 内部管理活跃波纹列表
};
```

**行为：**
- 点击时在点击位置生成圆形水波纹
- 波纹向外扩散同时透明度降低
- 动画时长：400ms（可配置）
- 缓动曲线：`QEasingCurve::OutCubic`
- 支持多波纹叠加（快速点击）
- 默认涟漪颜色：白色半透明（适配深色按钮）

### 3. 页面切换淡入淡出

**主窗口启动动画：**
- 使用 `QWidget::setWindowOpacity()`（Qt 6 推荐方式）
- 初始 opacity: 0 → 目标 opacity: 1
- 时长：300ms，缓动：OutCubic
- 在主窗口构造函数中启动，显示前完成

**对话框动画：**
- 打开：从 opacity 0 → 1，scale 0.95 → 1
- 关闭：从 opacity 1 → 0，scale 1 → 0.95
- 时长：250ms
- 使用 `QPropertyAnimation` 控制 opacity 和 scale

**与现有 UI 集成：**
- 登录对话框的"记住密码"复选框一同参与淡入动画
- 动画不影响按钮点击事件处理

### 4. 计时器数字滚动

创建自定义 `AnimatedLabel` 类继承 `QLabel`：

**公共接口：**
```cpp
class AnimatedLabel : public QLabel {
    Q_OBJECT

public:
    explicit AnimatedLabel(QWidget *parent = nullptr);

    // 设置文本并触发动画（如果值变化）
    void setAnimatedText(const QString &text);

    // 设置动画时长（毫秒）
    void setAnimationDuration(int msecs);

    // 立即设置文本（无动画）
    void setTextImmediate(const QString &text);

private slots:
    void onAnimationFinished();

private:
    int m_duration = 200;
    QString m_oldText;
    QString m_newText;
    // 内部使用 QPropertyAnimation 控制 y 偏移
};
```

**行为：**
- 调用 `setAnimatedText()` 时，若新值不同则触发动画
- 旧数字向上滑出（y -20px → -40px），新数字从下方滑入（y +20px → 0）
- 时长：200ms，缓动：OutQuad
- 动画期间双缓冲显示新旧两个值

**与现有代码集成：**
```cpp
// 在 MainWindow 中替换 elapsedTimeLabel
// 原代码：elapsedTimeLabel->setText("...")
// 新代码：elapsedTimeAnimatedLabel->setAnimatedText("...")
// updateElapsedTime() 槽函数中调用 setAnimatedText
```

## 技术实现

### 新建文件（放在 src/ 目录，遵循现有结构）

**src/ripplebutton.h / src/ripplebutton.cpp**
- 涟漪按钮自定义控件

**src/animatedlabel.h / src/animatedlabel.cpp**
- 数字滚动标签控件

### 修改文件

**src/mainwindow.cpp**
- 在主窗口构造函数中应用 QSS（`setStyleSheet()`）
- 替换普通按钮为 RippleButton（上下班按钮、同步按钮等）
- 添加窗口淡入动画（使用 `setWindowOpacity()`）
- 替换 `elapsedTimeLabel` 为 `AnimatedLabel`
- 在 `updateElapsedTime()` 中使用 `setAnimatedText()`

**src/logindialog.cpp**
- 应用 QSS 样式表
- 添加对话框弹出/关闭动画
- 替换按钮为 RippleButton（登录按钮、获取验证码按钮）
- 复选框保持原生样式（不参与涟漪）

**CMakeLists.txt**
- 添加 ripplebutton.cpp 和 animatedlabel.cpp 到编译

### QSS 应用方式

在主窗口和对话框构造函数中直接调用 `setStyleSheet()`：

```cpp
MainWindow::MainWindow(QWidget *parent) {
    // ... 其他初始化 ...
    setStyleSheet(R"(
        QMainWindow {
            border-radius: 16px;
        }
        QPushButton {
            border-radius: 12px;
            border: none;
        }
        QLineEdit {
            border-radius: 8px;
            padding: 8px 12px;
            border: 1px solid #ccc;
        }
        QListWidget::item {
            border-radius: 8px;
            margin: 4px;
        }
    )");
}
```

## 验收标准

- [ ] 所有窗口和按钮显示圆角边框
- [ ] 点击按钮时出现涟漪扩散效果
- [ ] 主窗口启动时淡入显示
- [ ] 对话框打开/关闭有淡入淡出动画
- [ ] 计时器数字变化时有滚动效果
- [ ] 动画流畅，60fps 无卡顿
- [ ] 不影响现有功能逻辑

## 性能考虑

- 使用 `QPropertyAnimation` 利用 GPU 加速
- 动画结束时及时清理资源
- 避免在动画中频繁重绘

### 禁用动画机制

使用简单的静态变量控制（不需要持久化设置）：

```cpp
// animationutils.h
class AnimationUtils {
public:
    static bool animationsEnabled();
    static void setAnimationsEnabled(bool enabled);
private:
    static bool s_enabled;
};

// RippleButton::mousePressEvent 中检查
if (!AnimationUtils::animationsEnabled()) {
    QPushButton::mousePressEvent(event);
    return;
}

// AnimatedLabel::setAnimatedText 中检查
if (!AnimationUtils::animationsEnabled()) {
    QLabel::setText(text);
    return;
}
```

**默认：**动画启用。如需禁用，在 main() 中调用 `AnimationUtils::setAnimationsEnabled(false)`。

## 测试策略

1. **单元测试**：测试 RippleButton 和 AnimatedLabel 的基础功能（禁用动画模式下）
2. **手动测试**：视觉确认动画流畅度（人眼观察 60fps）
3. **回归测试**：确保动画不影响按钮点击、数据保存等核心功能

- Qt 6.0+ 支持
- 不影响现有信号槽连接
- 向后兼容（无动画时正常显示）

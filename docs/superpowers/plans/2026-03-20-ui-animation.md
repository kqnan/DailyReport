# UI 动画与美化实现计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为工时记录应用添加圆角边框、按钮涟漪、页面淡入淡出、数字滚动等现代化 UI 动效。

**Architecture:** 创建两个自定义控件（RippleButton、AnimatedLabel），使用 Qt 的 QPropertyAnimation 和 QSS 实现动画效果，所有动画可通过 AnimationUtils 统一禁用。

**Tech Stack:** Qt 6.6 (QPropertyAnimation, QEasingCurve, QPainter), C++20

---

## 文件结构

**新建文件:**
- `src/ripplebutton.h/cpp`: 涟漪按钮控件，绘制扩散波纹动画
- `src/animatedlabel.h/cpp`: 数字滚动标签，实现上下滑动切换效果
- `src/animationutils.h/cpp`: 动画工具类，控制全局动画开关

**修改文件:**
- `src/mainwindow.h/cpp`: 应用 QSS 样式表，替换按钮为 RippleButton，添加窗口淡入动画，替换计时器标签为 AnimatedLabel
- `src/logindialog.h/cpp`: 应用 QSS 样式表，添加对话框动画，替换按钮为 RippleButton
- `CMakeLists.txt`: 添加新源文件到编译目标

---

## Chunk 1: 动画工具类和 QSS 样式

### Task 1: 创建 AnimationUtils 工具类

**Files:**
- Create: `src/animationutils.h`
- Create: `src/animationutils.cpp`

- [ ] **Step 1: 创建 animationutils.h 头文件**

创建 `src/animationutils.h`：

```cpp
#pragma once

class AnimationUtils {
public:
    static bool animationsEnabled();
    static void setAnimationsEnabled(bool enabled);
private:
    static bool s_enabled;
};
```

- [ ] **Step 2: 创建 animationutils.cpp 实现文件**

创建 `src/animationutils.cpp`：

```cpp
#include "animationutils.h"

bool AnimationUtils::s_enabled = true;

bool AnimationUtils::animationsEnabled() {
    return s_enabled;
}

void AnimationUtils::setAnimationsEnabled(bool enabled) {
    s_enabled = enabled;
}
```

- [ ] **Step 3: 编译验证**

Run: `cd /root/DailyReport/build && cmake --build . 2>&1 | tail -5`
Expected: Build succeeds with no errors

- [ ] **Step 4: Commit**

```bash
git add src/animationutils.h src/animationutils.cpp
git commit -m "feat: add AnimationUtils for controlling global animation state"
```

---

### Task 2: 在主窗口应用 QSS 圆角样式

**Files:**
- Modify: `src/mainwindow.cpp` (initUI 函数)

- [ ] **Step 1: 在 initUI 中添加 QSS 样式表**

在 `src/mainwindow.cpp` 的 `initUI()` 函数末尾（所有控件创建之后），`setCentralWidget(centralWidget)` 之前添加：

```cpp
    // Apply global QSS styles
    setStyleSheet(R"(
        QMainWindow {
            border-radius: 16px;
        }
        QPushButton {
            border-radius: 12px;
            border: none;
        }
        QPushButton:hover {
            opacity: 0.9;
        }
        QPushButton:pressed {
            opacity: 0.8;
        }
        QLineEdit, QDateEdit {
            border-radius: 8px;
            padding: 8px 12px;
            border: 1px solid #ccc;
        }
        QListWidget {
            border-radius: 8px;
        }
        QListWidget::item {
            border-radius: 8px;
            margin: 4px;
            padding: 8px;
        }
        QLabel {
            color: #333;
        }
    )");
```

- [ ] **Step 2: 移除硬编码样式，合并到 QSS**

将现有的按钮样式（lines 88-89）从：
```cpp
startButton->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #10b981, stop:1 #059669); color: white; padding: 16px 48px; font-size: 20px; border-radius: 12px;");
endButton->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ef4444, stop:1 #dc2626); color: white; padding: 16px 48px; font-size: 20px; border-radius: 12px;");
```

改为：
```cpp
startButton->setProperty("class", "primary-button");
endButton->setProperty("class", "danger-button");
```

并更新 QSS 样式表包含：
```cpp
    setStyleSheet(R"(
        QMainWindow {
            border-radius: 16px;
        }
        QPushButton {
            border-radius: 12px;
            border: none;
            padding: 16px 48px;
            font-size: 20px;
            color: white;
        }
        QPushButton[class="primary-button"] {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #10b981, stop:1 #059669);
        }
        QPushButton[class="danger-button"] {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ef4444, stop:1 #dc2626);
        }
        QPushButton:hover {
            opacity: 0.9;
        }
        QPushButton:pressed {
            opacity: 0.8;
        }
        QLineEdit, QDateEdit {
            border-radius: 8px;
            padding: 8px 12px;
            border: 1px solid #ccc;
        }
        QListWidget {
            border-radius: 8px;
        }
        QListWidget::item {
            border-radius: 8px;
            margin: 4px;
            padding: 8px;
        }
        QLabel {
            color: #333;
        }
    )");
```

- [ ] **Step 3: 构建并验证**

Run: `cd /root/DailyReport/build && cmake --build . 2>&1 | tail -5`
Expected: Build succeeds, no style regressions

- [ ] **Step 4: Commit**

```bash
git add src/mainwindow.cpp
git commit -m "feat: apply QSS styles for rounded corners and unified button styling"
```

---

## Chunk 2: 涟漪按钮控件

### Task 3: 创建 RippleButton 自定义控件

**Files:**
- Create: `src/ripplebutton.h`
- Create: `src/ripplebutton.cpp`

- [ ] **Step 1: 创建 ripplebutton.h 头文件**

创建 `src/ripplebutton.h`：

```cpp
#pragma once

#include <QPushButton>
#include <QColor>
#include <QList>
#include <QPropertyAnimation>

struct Ripple {
    QPoint center;
    qreal radius;
    qreal maxRadius;
    qreal opacity;
    QPropertyAnimation *animation;
};

class RippleButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QColor rippleColor READ rippleColor WRITE setRippleColor)
    Q_PROPERTY(int rippleDuration READ rippleDuration WRITE setRippleDuration)

public:
    explicit RippleButton(QWidget *parent = nullptr);
    explicit RippleButton(const QString &text, QWidget *parent = nullptr);
    ~RippleButton();

    void setRippleColor(const QColor &color);
    QColor rippleColor() const { return m_rippleColor; }

    void setRippleDuration(int msecs);
    int rippleDuration() const { return m_rippleDuration; }

    void setRippleEnabled(bool enabled);
    bool isRippleEnabled() const { return m_rippleEnabled; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateRipple();
    void removeRipple();

private:
    QColor m_rippleColor = QColor(255, 255, 255, 100);
    int m_rippleDuration = 400;
    bool m_rippleEnabled = true;
    QList<Ripple> m_ripples;
};
```

- [ ] **Step 2: 创建 ripplebutton.cpp 实现文件**

创建 `src/ripplebutton.cpp`：

```cpp
#include "ripplebutton.h"
#include "animationutils.h"
#include <QPainter>
#include <QMouseEvent>
#include <QEasingCurve>

RippleButton::RippleButton(QWidget *parent)
    : QPushButton(parent)
{
}

RippleButton::RippleButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
}

RippleButton::~RippleButton() {
    for (auto &ripple : m_ripples) {
        if (ripple.animation) {
            ripple.animation->stop();
            delete ripple.animation;
        }
    }
}

void RippleButton::setRippleColor(const QColor &color) {
    m_rippleColor = color;
}

void RippleButton::setRippleDuration(int msecs) {
    m_rippleDuration = msecs;
}

void RippleButton::setRippleEnabled(bool enabled) {
    m_rippleEnabled = enabled;
}

void RippleButton::mousePressEvent(QMouseEvent *event) {
    if (!m_rippleEnabled || !AnimationUtils::animationsEnabled()) {
        QPushButton::mousePressEvent(event);
        return;
    }

    Ripple ripple;
    ripple.center = event->pos();
    ripple.radius = 0;
    ripple.maxRadius = qMax(width(), height()) * 1.5;
    ripple.opacity = 1.0;
    ripple.animation = new QPropertyAnimation(this, "");
    ripple.animation->setDuration(m_rippleDuration);
    ripple.animation->setStartValue(0.0);
    ripple.animation->setEndValue(ripple.maxRadius);
    ripple.animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(ripple.animation, &QPropertyAnimation::valueChanged, this, [this, &ripple](const QVariant &value) {
        ripple.radius = value.toReal();
        ripple.opacity = 1.0 - (ripple.radius / ripple.maxRadius);
        update();
    });

    connect(ripple.animation, &QPropertyAnimation::finished, this, &RippleButton::removeRipple);

    m_ripples.append(ripple);
    ripple.animation->start();

    QPushButton::mousePressEvent(event);
}

void RippleButton::paintEvent(QPaintEvent *event) {
    QPushButton::paintEvent(event);

    if (m_ripples.isEmpty()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const auto &ripple : m_ripples) {
        if (ripple.radius <= 0) continue;

        QColor color = m_rippleColor;
        color.setAlphaF(color.alphaF() * ripple.opacity);

        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(ripple.center, ripple.radius, ripple.radius);
    }
}

void RippleButton::updateRipple() {
    update();
}

void RippleButton::removeRipple() {
    auto *animation = qobject_cast<QPropertyAnimation*>(sender());
    if (!animation) return;

    for (int i = 0; i < m_ripples.size(); ++i) {
        if (m_ripples[i].animation == animation) {
            delete m_ripples[i].animation;
            m_ripples.removeAt(i);
            break;
        }
    }
    update();
}
```

- [ ] **Step 3: 修改 CMakeLists.txt 添加新文件**

在 `CMakeLists.txt` 中找到 `add_executable(DailyReport` 部分，在源文件列表中添加：

```cmake
add_executable(DailyReport
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/datamodel.cpp
    src/datamodel.h
    src/sessionmanager.cpp
    src/sessionmanager.h
    src/apimanager.cpp
    src/apimanager.h
    src/logindialog.cpp
    src/logindialog.h
    src/cloudsessionmanager.cpp
    src/cloudsessionmanager.h
    src/utils.cpp
    src/utils.h
    src/ripplebutton.cpp
    src/ripplebutton.h
    src/animatedlabel.cpp
    src/animatedlabel.h
    src/animationutils.cpp
    src/animationutils.h
)
```

- [ ] **Step 4: 构建验证**

Run: `cd /root/DailyReport/build && cmake .. && cmake --build . 2>&1 | tail -5`
Expected: Build succeeds, RippleButton compiles

- [ ] **Step 5: Commit**

```bash
git add src/ripplebutton.h src/ripplebutton.cpp CMakeLists.txt
git commit -m "feat: add RippleButton custom control with ripple animation"
```

---

## Chunk 3: 数字滚动标签

### Task 4: 创建 AnimatedLabel 自定义控件

**Files:**
- Create: `src/animatedlabel.h`
- Create: `src/animatedlabel.cpp`

- [ ] **Step 1: 创建 animatedlabel.h 头文件**

创建 `src/animatedlabel.h`：

```cpp
#pragma once

#include <QLabel>
#include <QPropertyAnimation>
#include <QString>

class AnimatedLabel : public QLabel {
    Q_OBJECT
    Q_PROPERTY(qreal slideOffset READ slideOffset WRITE setSlideOffset)

public:
    explicit AnimatedLabel(QWidget *parent = nullptr);

    void setAnimatedText(const QString &text);
    void setAnimationDuration(int msecs);
    void setTextImmediate(const QString &text);

    qreal slideOffset() const { return m_slideOffset; }
    void setSlideOffset(qreal offset);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onAnimationFinished();

private:
    int m_duration = 200;
    QString m_oldText;
    QString m_newText;
    qreal m_slideOffset = 0;
    bool m_isAnimating = false;
    QPropertyAnimation *m_animation;
};
```

- [ ] **Step 2: 创建 animatedlabel.cpp 实现文件**

创建 `src/animatedlabel.cpp`：

```cpp
#include "animatedlabel.h"
#include "animationutils.h"
#include <QPainter>
#include <QEasingCurve>

AnimatedLabel::AnimatedLabel(QWidget *parent)
    : QLabel(parent)
    , m_animation(new QPropertyAnimation(this, "slideOffset", this))
{
    m_animation->setDuration(m_duration);
    m_animation->setEasingCurve(QEasingCurve::OutQuad);
    connect(m_animation, &QPropertyAnimation::finished, this, &AnimatedLabel::onAnimationFinished);
}

void AnimatedLabel::setAnimatedText(const QString &text) {
    if (text == m_newText || text == this->text()) {
        return;
    }

    if (!AnimationUtils::animationsEnabled()) {
        setText(text);
        m_newText = text;
        m_oldText = text;
        return;
    }

    if (m_isAnimating) {
        m_animation->stop();
        m_oldText = m_newText;
    } else {
        m_oldText = this->text();
    }

    m_newText = text;
    m_isAnimating = true;
    m_slideOffset = 0;

    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->start();

    update();
}

void AnimatedLabel::setAnimationDuration(int msecs) {
    m_duration = msecs;
    m_animation->setDuration(msecs);
}

void AnimatedLabel::setTextImmediate(const QString &text) {
    m_animation->stop();
    m_isAnimating = false;
    m_oldText = text;
    m_newText = text;
    setText(text);
}

void AnimatedLabel::setSlideOffset(qreal offset) {
    m_slideOffset = offset;
    update();
}

void AnimatedLabel::paintEvent(QPaintEvent *event) {
    if (!m_isAnimating) {
        QLabel::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font = this->font();
    painter.setFont(font);

    QRect rect = this->rect();
    int yOffset = rect.height() * m_slideOffset;

    // Draw old text sliding up
    painter.setPen(QPen(palette().color(foregroundRole())));
    painter.drawText(rect.translated(0, -yOffset), Qt::AlignCenter, m_oldText);

    // Draw new text sliding in from below
    painter.setPen(QPen(palette().color(foregroundRole())));
    painter.drawText(rect.translated(0, rect.height() - yOffset), Qt::AlignCenter, m_newText);
}

void AnimatedLabel::onAnimationFinished() {
    m_isAnimating = false;
    setText(m_newText);
    m_oldText = m_newText;
    update();
}
```

- [ ] **Step 3: 构建验证**

Run: `cd /root/DailyReport/build && cmake --build . 2>&1 | tail -5`
Expected: Build succeeds

- [ ] **Step 4: Commit**

```bash
git add src/animatedlabel.h src/animatedlabel.cpp
git commit -m "feat: add AnimatedLabel with number scrolling animation"
```

---

## Chunk 4: 集成动画到主窗口

### Task 5: 在主窗口中使用 RippleButton

**Files:**
- Modify: `src/mainwindow.h` (添加 RippleButton 头文件，修改按钮类型)
- Modify: `src/mainwindow.cpp` (替换按钮创建代码)

- [ ] **Step 1: 修改 mainwindow.h 添加 RippleButton**

在 `src/mainwindow.h` 中：
1. 添加 `#include "ripplebutton.h"`
2. 将所有 `QPushButton *` 改为 `RippleButton *`（除了 editButton, deleteButton 保持原样）

```cpp
#include "ripplebutton.h"

// ...

private:
    // Buttons
    RippleButton *startButton;
    RippleButton *endButton;
    QPushButton *editButton;      // Keep as QPushButton
    QPushButton *deleteButton;    // Keep as QPushButton
    RippleButton *exportCsvButton;
    RippleButton *exportJsonButton;
    RippleButton *todayButton;
    RippleButton *openFolderButton;
    RippleButton *loginButton;
    RippleButton *syncButton;
```

- [ ] **Step 2: 修改 mainwindow.cpp 使用 RippleButton**

将按钮创建代码从：
```cpp
startButton = new QPushButton("🟢 上班");
endButton = new QPushButton("🔴 下班");
```

改为：
```cpp
startButton = new RippleButton("🟢 上班");
endButton = new RippleButton("🔴 下班");
exportCsvButton = new RippleButton("📥 导出 CSV");
exportJsonButton = new RippleButton("📥 导出 JSON");
todayButton = new RippleButton("今天");
openFolderButton = new RippleButton("📂 打开记录文件夹");
loginButton = new RippleButton("🔐 登录");
syncButton = new RippleButton("📤 同步");
```

- [ ] **Step 3: 构建并验证**

Run: `cd /root/DailyReport/build && cmake --build . 2>&1 | tail -5`
Expected: Build succeeds

- [ ] **Step 4: Commit**

```bash
git add src/mainwindow.h src/mainwindow.cpp
git commit -m "feat: replace QPushButton with RippleButton in main window"
```

---

### Task 6: 在主窗口中使用 AnimatedLabel 并添加淡入动画

**Files:**
- Modify: `src/mainwindow.h` (添加 AnimatedLabel 头文件)
- Modify: `src/mainwindow.cpp` (替换标签，添加淡入动画)

- [ ] **Step 1: 修改 mainwindow.h 添加 AnimatedLabel**

在 `src/mainwindow.h` 中：
1. 添加 `#include "animatedlabel.h"`
2. 将 `elapsedTimeLabel` 从 `QLabel*` 改为 `AnimatedLabel*`

```cpp
#include "animatedlabel.h"

// ...

private:
    // Labels
    QLabel *totalHoursLabel;
    QLabel *sessionCountLabel;
    QLabel *startTimeLabel;
    QLabel *statusLabel;
    AnimatedLabel *elapsedTimeLabel;  // Changed from QLabel
```

- [ ] **Step 2: 修改 mainwindow.cpp 使用 AnimatedLabel 并添加淡入动画**

在 `initUI()` 中修改：
```cpp
elapsedTimeLabel = new AnimatedLabel();  // Changed from QLabel
elapsedTimeLabel->setAnimationDuration(200);
elapsedTimeLabel->hide();
```

在构造函数末尾（`initUI()` 调用之后）添加窗口淡入动画：

```cpp
    // Window fade-in animation
    if (AnimationUtils::animationsEnabled()) {
        setWindowOpacity(0.0);
        QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "windowOpacity");
        fadeIn->setDuration(300);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    }
```

- [ ] **Step 3: 修改 updateElapsedTime 使用 setAnimatedText**

找到 `updateElapsedTime()` 函数，将：
```cpp
elapsedTimeLabel->setText(QString("已工作：%1 小时 %2 分钟").arg(hours).arg(minutes));
```

改为：
```cpp
elapsedTimeLabel->setAnimatedText(QString("已工作：%1 小时 %2 分钟").arg(hours).arg(minutes));
```

- [ ] **Step 4: 添加必要的 includes**

在 `src/mainwindow.cpp` 顶部添加：
```cpp
#include "animationutils.h"
#include <QPropertyAnimation>
```

- [ ] **Step 5: 构建并验证**

Run: `cd /root/DailyReport/build && cmake --build . 2>&1 | tail -5`
Expected: Build succeeds

- [ ] **Step 6: Commit**

```bash
git add src/mainwindow.h src/mainwindow.cpp
git commit -m "feat: add AnimatedLabel and window fade-in animation"
```

---

## Chunk 5: 集成动画到登录对话框

### Task 7: 在登录对话框中应用样式和动画

**Files:**
- Modify: `src/logindialog.cpp`

- [ ] **Step 1: 应用 QSS 样式表**

在 `src/logindialog.cpp` 的构造函数中，所有控件创建之后添加：

```cpp
    // Apply QSS styles
    setStyleSheet(R"(
        QDialog {
            border-radius: 12px;
        }
        QLineEdit {
            border-radius: 8px;
            padding: 8px 12px;
            border: 1px solid #ccc;
        }
        QPushButton {
            border-radius: 8px;
            padding: 8px 16px;
        }
    )");
```

- [ ] **Step 2: 添加对话框淡入动画**

在构造函数末尾添加：

```cpp
    // Dialog fade-in animation
    if (AnimationUtils::animationsEnabled()) {
        setWindowOpacity(0.0);
        QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "windowOpacity");
        fadeIn->setDuration(250);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    }
```

- [ ] **Step 3: 添加必要的 includes**

在 `src/logindialog.cpp` 顶部添加：
```cpp
#include "animationutils.h"
#include <QPropertyAnimation>
```

- [ ] **Step 4: 构建并验证**

Run: `cd /root/DailyReport/build && cmake --build . 2>&1 | tail -5`
Expected: Build succeeds

- [ ] **Step 5: Commit**

```bash
git add src/logindialog.cpp
git commit -m "feat: apply QSS styles and fade-in animation to login dialog"
```

---

## Chunk 6: 添加单元测试

### Task 8: 为动画控件添加单元测试

**Files:**
- Create: `tests/test_ripplebutton.cpp`
- Create: `tests/test_animatedlabel.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 创建 RippleButton 测试文件**

创建 `tests/test_ripplebutton.cpp`：

```cpp
#include <QTest>
#include <QMouseEvent>
#include "../src/ripplebutton.h"
#include "../src/animationutils.h"

class TestRippleButton : public QObject {
    Q_OBJECT

private slots:
    void testRippleDisabled() {
        AnimationUtils::setAnimationsEnabled(false);

        RippleButton button("Test");
        button.show();

        // Simulate mouse press - should not crash
        QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(50, 25), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&button, &pressEvent);

        QVERIFY(button.isEnabled());
    }

    void testRippleProperties() {
        RippleButton button;

        // Test default values
        QCOMPARE(button.rippleDuration(), 400);
        QVERIFY(button.isRippleEnabled());

        // Test setters
        button.setRippleDuration(200);
        QCOMPARE(button.rippleDuration(), 200);

        button.setRippleEnabled(false);
        QVERIFY(!button.isRippleEnabled());

        button.setRippleColor(QColor(255, 0, 0, 50));
        QCOMPARE(button.rippleColor(), QColor(255, 0, 0, 50));
    }
};

QTEST_MAIN(TestRippleButton)
#include "test_ripplebutton.moc"
```

- [ ] **Step 2: 创建 AnimatedLabel 测试文件**

创建 `tests/test_animatedlabel.cpp`：

```cpp
#include <QTest>
#include "../src/animatedlabel.h"
#include "../src/animationutils.h"

class TestAnimatedLabel : public QObject {
    Q_OBJECT

private slots:
    void testAnimationDisabled() {
        AnimationUtils::setAnimationsEnabled(false);

        AnimatedLabel label;
        label.setTextImmediate("Initial");

        label.setAnimatedText("New Text");
        QCOMPARE(label.text(), QString("New Text"));
    }

    void testSetTextImmediate() {
        AnimatedLabel label;

        label.setTextImmediate("Test");
        QCOMPARE(label.text(), QString("Test"));
    }

    void testDuration() {
        AnimatedLabel label;
        label.setAnimationDuration(500);
        // Verify no crash
        label.setAnimatedText("Test");
    }
};

QTEST_MAIN(TestAnimatedLabel)
#include "test_animatedlabel.moc"
```

- [ ] **Step 3: 修改 CMakeLists.txt 添加测试**

在 `CMakeLists.txt` 末尾添加：

```cmake
add_executable(test_ripplebutton
    tests/test_ripplebutton.cpp
    src/ripplebutton.h
    src/ripplebutton.cpp
    src/animationutils.h
    src/animationutils.cpp
)

target_link_libraries(test_ripplebutton PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Test
)

add_executable(test_animatedlabel
    tests/test_animatedlabel.cpp
    src/animatedlabel.h
    src/animatedlabel.cpp
    src/animationutils.h
    src/animationutils.cpp
)

target_link_libraries(test_animatedlabel PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Test
)
```

- [ ] **Step 4: 构建并运行测试**

Run: `cd /root/DailyReport/build && cmake .. && cmake --build . 2>&1 | tail -10`
Expected: Build succeeds, test executables created

Run: `QT_QPA_PLATFORM=offscreen ./test_ripplebutton && ./test_animatedlabel`
Expected: All tests pass

- [ ] **Step 5: Commit**

```bash
git add tests/test_ripplebutton.cpp tests/test_animatedlabel.cpp CMakeLists.txt
git commit -m "test: add unit tests for RippleButton and AnimatedLabel"
```

---

## 验证清单

- [ ] AnimationUtils 可以启用/禁用全局动画
- [ ] 所有窗口和按钮显示圆角边框
- [ ] 点击 RippleButton 时出现涟漪扩散效果
- [ ] 主窗口启动时有淡入动画
- [ ] 对话框打开时有淡入动画
- [ ] 计时器数字变化时有滚动动画
- [ ] 禁用动画时所有功能正常（无动画但仍可用）
- [ ] 所有单元测试通过

---

## 提交历史（预期）

1. `feat: add AnimationUtils for controlling global animation state`
2. `feat: apply QSS styles for rounded corners and unified button styling`
3. `feat: add RippleButton custom control with ripple animation`
4. `feat: add AnimatedLabel with number scrolling animation`
5. `feat: replace QPushButton with RippleButton in main window`
6. `feat: add AnimatedLabel and window fade-in animation`
7. `feat: apply QSS styles and fade-in animation to login dialog`
8. `test: add unit tests for RippleButton and AnimatedLabel`

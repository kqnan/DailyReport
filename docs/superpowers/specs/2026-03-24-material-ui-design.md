# Material Design UI 美化设计规范

## 概述

将 DailyReport 应用的 UI 美化为 Material Design 风格，使用莫兰迪色系，包含视觉样式和动效。

## 设计决策

| 决策项 | 选择 |
|--------|------|
| 实现方案 | 纯 QSS + QPropertyAnimation |
| 覆盖范围 | 全部界面（主窗口 + 登录对话框） |
| 动效级别 | 完整动效 |
| 动画开关 | 不需要 |
| 配色风格 | 莫兰迪色系（鲜明版） |

## 配色方案

### 莫兰迪色系

低饱和度、灰调、柔和、高级感，并提高对比度确保辨识度。

| 角色 | 颜色名 | 值 | 使用场景 |
|------|--------|-----|---------|
| **Primary** | 鼠尾草绿 | `#6B8E6B` | 上班按钮、焦点边框 |
| **Danger** | 玫瑰粉 | `#B87A7A` | 下班、删除按钮 |
| **Secondary** | 雾蓝 | `#6B8BA3` | 登录、获取验证码、同步 |
| **Accent** | 薰衣草 | `#8B7FA3` | 悬停、选中状态 |
| **Background** | 暖灰 | `#F0EDE8` | 窗口背景 |
| **Surface** | 米白 | `#FAF8F5` | 卡片、输入框、列表 |
| **Border** | 灰棕 | `#C8C2B8` | 输入框、列表边框 |
| **Text Primary** | 深棕灰 | `#2D2D2D` | 主要文字 |
| **Text Secondary** | 暖灰 | `#5A5A5A` | 次要文字 |

### 按钮配色

| 按钮 | 颜色 | 值 |
|------|------|-----|
| 上班 | 鼠尾草绿 | `#6B8E6B` |
| 下班 | 玫瑰粉 | `#B87A7A` |
| 登录/同步 | 雾蓝 | `#6B8BA3` |
| 删除 | 玫瑰粉 | `#B87A7A` |
| 其他 | 薰衣草 | `#8B7FA3` |

## 圆角规范

| 元素 | 圆角 |
|------|------|
| 窗口 | 16px |
| 按钮 | 12px |
| 输入框/卡片 | 8px |

## 按钮尺寸

保持现有尺寸不变，只应用样式。

- 上班/下班按钮：`padding: 16px 48px; font-size: 20px`
- 其他按钮：Qt 默认大小

## 动效设计

| 动效 | 实现方式 | 时长 | 应用位置 |
|------|----------|------|----------|
| 涟漪效果 | `paintEvent` + `QPropertyAnimation` | 400ms | 所有按钮点击 |
| 窗口淡入 | `windowOpacity` 动画 | 300ms | 主窗口、登录对话框启动 |
| 数字滚动 | 双缓冲 + `slideOffset` | 200ms | 工作时长数字变化 |
| 列表项滑入 | `geometry` 动画 | 150ms | 会话列表加载 |

**动画曲线：** `QEasingCurve::OutCubic`

## 文件结构

```
src/
├── animationutils.h/cpp    # 全局动画控制
├── ripplebutton.h/cpp      # 涟漪按钮控件
├── animatedlabel.h/cpp     # 数字滚动标签
├── mainwindow.h/cpp        # 应用样式
└── logindialog.h/cpp       # 应用样式
```

## 组件设计

### AnimationUtils

```cpp
class AnimationUtils {
public:
    static bool animationsEnabled();
    static void setAnimationsEnabled(bool enabled);
private:
    static bool s_enabled;
};
```

### RippleButton

继承 `QPushButton`，重写：
- `mousePressEvent` - 记录点击位置，创建涟漪
- `paintEvent` - 绘制涟漪效果
- `Ripple` 结构体 - center, radius, maxRadius, opacity, animation

### AnimatedLabel

继承 `QLabel`，重写：
- `paintEvent` - 双缓冲绘制滚动效果
- `setAnimatedText` - 启动滚动动画
- `slideOffset` 属性 - 控制滚动位置

## QSS 样式示例

```css
/* 窗口背景 */
QMainWindow { background-color: #F0EDE8; }

/* 按钮 */
QPushButton {
    border-radius: 12px;
    font-weight: bold;
    color: white;
}
QPushButton#primary { background-color: #6B8E6B; }
QPushButton#danger { background-color: #B87A7A; }
QPushButton#secondary { background-color: #6B8BA3; }
QPushButton#accent { background-color: #8B7FA3; }

/* 输入框 */
QLineEdit {
    border: 1px solid #C8C2B8;
    border-radius: 8px;
    padding: 8px;
    background: #FAF8F5;
    color: #2D2D2D;
}
QLineEdit:focus { border: 2px solid #6B8E6B; }

/* 列表 */
QListWidget {
    border: 1px solid #C8C2B8;
    border-radius: 8px;
    background: #FAF8F5;
}

/* 标签 */
QLabel { color: #2D2D2D; }
QLabel#secondary { color: #5A5A5A; }
```

## 验收标准

1. 所有按钮点击时有涟漪动画
2. 主窗口和登录对话框启动时有淡入效果
3. 工作时长数字变化时有滚动动画
4. 视觉风格统一，使用莫兰迪色系
5. 构建通过：Linux、Windows、macOS
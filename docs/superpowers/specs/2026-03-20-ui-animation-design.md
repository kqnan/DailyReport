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

**行为：**
- 点击时在点击位置生成圆形水波纹
- 波纹向外扩散同时透明度降低
- 动画时长：400ms
- 缓动曲线：`QEasingCurve::OutCubic`
- 支持多波纹叠加（快速点击）

**实现：**
```cpp
class RippleButton : public QPushButton {
    // 使用 QPropertyAnimation 控制波纹缩放和透明度
    // 重写 mousePressEvent 获取点击位置
    // 使用 QPainter 在按钮上绘制波纹
};
```

### 3. 页面切换淡入淡出

**主窗口启动动画：**
- 初始 opacity: 0
- 目标 opacity: 1
- 时长：300ms
- 缓动：OutCubic

**对话框动画：**
- 打开：从 opacity 0 → 1，scale 0.95 → 1
- 关闭：从 opacity 1 → 0，scale 1 → 0.95
- 时长：250ms

**实现方式：**
使用 `QGraphicsOpacityEffect` + `QPropertyAnimation`

### 4. 计时器数字滚动

创建自定义 `AnimatedLabel` 类继承 `QLabel`：

**行为：**
- 数字变化时触发滚动动画
- 旧数字向上滑出，新数字从下方滑入
- 时长：200ms
- 缓动：OutQuad

**实现：**
```cpp
class AnimatedLabel : public QLabel {
    // 使用 QPropertyAnimation 控制 y 偏移
    // 双缓冲：同时显示旧值和新值
    // 使用 QPainter 绘制文字
};
```

## 技术实现

### 新建文件

**src/widgets/ripplebutton.h/cpp**
- 涟漪按钮自定义控件

**src/widgets/animatedlabel.h/cpp**
- 数字滚动标签控件

**src/utils/animationutils.h/cpp**（可选）
- 动画工具函数封装

### 修改文件

**src/mainwindow.cpp**
- 应用 QSS 样式表
- 替换普通按钮为 RippleButton
- 添加窗口淡入动画
- 替换时间标签为 AnimatedLabel

**src/logindialog.cpp**
- 应用 QSS 样式表
- 添加对话框弹出/关闭动画
- 替换按钮为 RippleButton

**CMakeLists.txt**
- 添加新 widget 文件到编译

### QSS 样式表示例

```css
/* 全局圆角 */
QMainWindow, QDialog {
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
- 支持禁用动画（低性能设备）

## 兼容性

- Qt 6.0+ 支持
- 不影响现有信号槽连接
- 向后兼容（无动画时正常显示）

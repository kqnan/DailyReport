# 主窗口布局简化设计

## 概述

简化主窗口布局，移除冗余按钮，将登录状态整合到标题栏，使界面更清爽。

## 目标布局

```
┌─────────────────────────────────────┐
│ 工时记录            未登录 [登录] [X] │
├─────────────────────────────────────┤
│                                     │
│  今日已工作: 0分钟    上班片段: 0段  │
│                                     │
│         ┌─────────┐                 │
│         │  上班   │                 │
│         └─────────┘                 │
│         开始时间: -                 │
│                                     │
├─────────────────────────────────────┤
│ 📅 选择日期: [3/24/26 ▼] [今天]     │
├─────────────────────────────────────┤
│ 当日记录:                            │
│ ┌─────────────────────────────────┐ │
│ │                                 │ │
│ │      暂无记录                   │ │
│ │                                 │ │
│ │ 点击上方"上班"按钮开始记录      │ │
│ │                                 │ │
│ └─────────────────────────────────┘ │
└─────────────────────────────────────┘
```

## 改动清单

### 1. 隐藏按钮

在 `initUI()` 中调用 `hide()` 隐藏以下按钮（保留功能代码）：

| 按钮 | 变量名 | 处理方式 |
|------|--------|----------|
| 导出 CSV | `exportCsvButton` | hide() |
| 导出 JSON | `exportJsonButton` | hide() |
| 打开记录文件夹 | `openFolderButton` | hide() |
| 同步 | `syncButton` | hide() |

### 2. 登录状态移至标题栏

将 `statusLabel` 和 `loginButton` 移到窗口标题区域右侧。

**实现方式：** 使用自定义标题栏或调整布局。

**简化方案：** 保持当前布局结构，仅隐藏导出/同步按钮行，将状态行合并到日期选择行。

### 3. 新增"当日记录"标签

在 `sessionListWidget` 上方添加标签：

```cpp
QLabel *recordTitleLabel = new QLabel("当日记录:");
```

## 代码修改

### mainwindow.h

无新增成员变量。

### mainwindow.cpp initUI()

1. 在按钮创建后添加 `hide()` 调用
2. 移除 `exportLayout` 布局行
3. 添加 `recordTitleLabel` 并在 `sessionListWidget` 前
4. 调整布局间距

## 布局结构对比

### Before

```
mainLayout
├── headerLayout (今日统计)
├── controlLayout (上班/下班按钮)
├── timeLayout (开始时间)
├── elapsedLayout (已工作时间)
├── dateLayout (日期选择)
├── exportLayout (导出CSV/JSON/打开文件夹/同步)  ← 删除
├── statusLayout (状态+登录)
└── sessionListWidget
```

### After

```
mainLayout
├── headerLayout (今日统计)
├── controlLayout (上班/下班按钮)
├── timeLayout (开始时间)
├── elapsedLayout (已工作时间)
├── dateLayout (日期选择)
├── recordTitleLabel (当日记录:)  ← 新增
└── sessionListWidget
```

**注意：** `statusLayout` 的处理待定，当前方案保留在底部或合并到日期行。

## 验收标准

1. 界面显示符合目标布局
2. 隐藏的按钮功能代码保留
3. 所有原有功能正常工作
4. 编译无警告
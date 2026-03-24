# 主窗口布局简化设计

## 概述

简化主窗口布局，隐藏冗余按钮（导出、同步），新增"当日记录"标签，使界面更清爽。

## 目标布局

```
┌─────────────────────────────────────┐
│ 工时记录                    [_][□][X] │
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
│ 状态: 未登录              [登录]     │
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

### 2. 保留登录状态行

保留 `statusLabel` 和 `loginButton` 在日期选择下方，不改变其位置。

### 3. 新增"当日记录"标签

在 `sessionListWidget` 上方添加标签：

```cpp
QLabel *recordTitleLabel = new QLabel("当日记录:");
```

## 代码修改

### mainwindow.h

无新增成员变量。

### mainwindow.cpp initUI()

#### 修改点 1：隐藏按钮（第 205-250 行附近）

在按钮创建后立即调用 `hide()`：

```cpp
exportCsvButton = new QPushButton("导出 CSV");
exportCsvButton->hide();  // 新增

exportJsonButton = new QPushButton("导出 JSON");
exportJsonButton->hide();  // 新增

openFolderButton = new QPushButton("打开记录文件夹");
openFolderButton->hide();  // 新增

// syncButton 创建后
syncButton->hide();  // 新增
```

#### 修改点 2：删除 exportLayout（第 319-326 行）

删除以下代码：

```cpp
// 删除这段
QHBoxLayout *exportLayout = new QHBoxLayout();
exportLayout->addWidget(exportCsvButton);
exportLayout->addWidget(exportJsonButton);
exportLayout->addWidget(openFolderButton);
exportLayout->addWidget(syncButton);
exportLayout->addStretch();
mainLayout->addLayout(exportLayout);
```

#### 修改点 3：新增当日记录标签（在 sessionListWidget 前）

```cpp
// 在 mainLayout->addWidget(sessionListWidget); 前添加
QLabel *recordTitleLabel = new QLabel("当日记录:");
recordTitleLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 8px;");
mainLayout->addWidget(recordTitleLabel);
```

#### 修改点 4：保留信号连接

隐藏按钮不影响功能，信号连接保持不变。

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
├── statusLayout (状态+登录)           ← 保留
├── recordTitleLabel (当日记录:)       ← 新增
└── sessionListWidget
```

**变更说明：** 仅删除 `exportLayout` 行，新增 `recordTitleLabel`。

## 验收标准

1. ✅ 导出 CSV/JSON、打开文件夹、同步按钮不可见（已隐藏）
2. ✅ 新增"当日记录:"标签显示在列表上方
3. ✅ 登录状态行保留在日期选择下方
4. ✅ 所有原有功能正常工作（上班、下班、登录、日期选择等）
5. ✅ 编译无警告
6. ✅ 应用启动后界面符合目标布局图
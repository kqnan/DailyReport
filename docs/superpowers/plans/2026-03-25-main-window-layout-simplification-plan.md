# 主窗口布局简化实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 简化主窗口布局，隐藏导出/同步按钮，新增"当日记录"标签

**Architecture:** 修改 mainwindow.cpp 的 initUI() 函数，隐藏按钮、删除布局行、添加新标签

**Tech Stack:** Qt 6.6 Widgets, C++20

---

## 文件结构

| 文件 | 操作 | 说明 |
|------|------|------|
| `src/mainwindow.cpp` | 修改 | 隐藏按钮、删除布局、添加标签 |

---

### Task 1: 隐藏导出和同步按钮

**Files:**
- Modify: `src/mainwindow.cpp:205-250`

- [ ] **Step 1: 隐藏 exportCsvButton**

在 `exportCsvButton = new QPushButton("导出 CSV");` 后添加 `hide()` 调用：

```cpp
exportCsvButton = new QPushButton("导出 CSV");
exportCsvButton->hide();
```

- [ ] **Step 2: 隐藏 exportJsonButton**

在 `exportJsonButton = new QPushButton("导出 JSON");` 后添加 `hide()` 调用：

```cpp
exportJsonButton = new QPushButton("导出 JSON");
exportJsonButton->hide();
```

- [ ] **Step 3: 隐藏 openFolderButton**

在 `openFolderButton = new QPushButton("打开记录文件夹");` 后添加 `hide()` 调用：

```cpp
openFolderButton = new QPushButton("打开记录文件夹");
openFolderButton->hide();
```

- [ ] **Step 4: 隐藏 syncButton**

在 `syncButton` 创建和样式设置后添加 `hide()` 调用：

```cpp
syncButton->setRippleColor(QColor(255, 255, 255, 60));
syncButton->hide();
```

---

### Task 2: 删除 exportLayout 布局行

**Files:**
- Modify: `src/mainwindow.cpp:319-326`

- [ ] **Step 1: 删除 exportLayout 代码块**

删除以下代码：

```cpp
// Export buttons
QHBoxLayout *exportLayout = new QHBoxLayout();
exportLayout->addWidget(exportCsvButton);
exportLayout->addWidget(exportJsonButton);
exportLayout->addWidget(openFolderButton);
exportLayout->addWidget(syncButton);
exportLayout->addStretch();
mainLayout->addLayout(exportLayout);
```

---

### Task 3: 新增"当日记录"标签

**Files:**
- Modify: `src/mainwindow.cpp:335` (sessionListWidget 添加位置)

- [ ] **Step 1: 在 sessionListWidget 前添加标签**

找到 `mainLayout->addWidget(sessionListWidget);` 行，在其前面添加：

```cpp
// Record title
QLabel *recordTitleLabel = new QLabel("当日记录:");
recordTitleLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 8px;");
mainLayout->addWidget(recordTitleLabel);
```

---

### Task 4: 构建验证

**Files:**
- 无文件修改，仅验证

- [ ] **Step 1: 构建项目**

```bash
cd /root/DailyReport/build && cmake --build .
```

Expected: 构建成功，无警告

- [ ] **Step 2: 运行应用验证 UI**

```bash
./DailyReport
```

Expected:
1. 界面不显示导出 CSV/JSON、打开文件夹、同步按钮
2. 列表上方显示"当日记录:"标签
3. 登录状态行正常显示

- [ ] **Step 3: 提交更改**

```bash
git add src/mainwindow.cpp
git commit -m "refactor: simplify main window layout - hide export/sync buttons, add record title"
```

---

## 验收清单

- [ ] 导出 CSV/JSON、打开文件夹、同步按钮不可见
- [ ] "当日记录:"标签显示在列表上方
- [ ] 登录状态行保留在日期选择下方
- [ ] 所有原有功能正常工作
- [ ] 编译无警告
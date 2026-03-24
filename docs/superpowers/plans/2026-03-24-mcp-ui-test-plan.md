# MCP UI 测试实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 创建 UI 测试提示词文档，用子代理执行 MCP 测试，截图保存到指定目录

**Architecture:** 创建提示词 markdown 文档，子代理读取该文档执行测试

**Tech Stack:** MCP tools (computer-use-offscreen), 子代理

---

## 文件结构

```
/root/DailyReport/tests/ui/
├── README.md                    # 测试说明
├── ui-test-prompt.md            # 测试提示词（子代理执行）
└── screenshots/                  # 截图目录（自动创建）
    └── {timestamp}/              # 按时间戳组织
        ├── 01_main_window.png
        ├── 02_login_dialog.png
        └── ...
```

---

### Task 1: 创建测试目录结构

**Files:**
- Create: `/root/DailyReport/tests/ui/README.md`
- Create: `/root/DailyReport/tests/ui/ui-test-prompt.md`

- [ ] **Step 1: 创建目录**

```bash
mkdir -p /root/DailyReport/tests/ui/screenshots
```

- [ ] **Step 2: 创建 README.md**

```markdown
# UI 自动化测试

使用 MCP computer-use-offscreen 工具进行 DailyReport 应用的 UI 自动化测试。

## 文件说明

- `ui-test-prompt.md` - 测试提示词，用于启动子代理执行测试
- `screenshots/` - 测试截图保存目录

## 执行测试

在 Claude Code 中启动子代理执行测试：

```
请阅读 tests/ui/ui-test-prompt.md 并执行其中的 UI 测试步骤
```

## 测试流程

1. 创建虚拟显示器会话
2. 启动 DailyReport 应用
3. 等待主窗口 → 截图
4. 点击登录按钮 → 截图
5. 等待登录对话框 → 截图
6. 输入账号 → 截图
7. 输入密码 → 截图
8. 点击登录 → 截图
9. 销毁会话

## 截图位置

截图保存在 `screenshots/{timestamp}/` 目录下。
```

- [ ] **Step 3: 提交**

```bash
git add tests/ui/
git commit -m "test: add UI test directory and README"
```

---

### Task 2: 创建测试提示词文档

**Files:**
- Create: `/root/DailyReport/tests/ui/ui-test-prompt.md`

- [ ] **Step 1: 创建提示词文档**

```markdown
# DailyReport UI 测试提示词

你正在执行 DailyReport 应用的 UI 自动化测试。

## 测试目标

测试登录流程：主界面 → 点击登录 → 输入账号密码 → 登录

## 配置参数

- 应用路径: `/root/DailyReport/build/DailyReport`
- 截图目录: `/root/DailyReport/tests/ui/screenshots/{当前时间戳}/`
- 默认超时: 10 秒

## 测试账号

从环境变量读取：
- `TEST_USERNAME` - 用户名
- `TEST_PASSWORD` - 密码

如果环境变量未设置，询问用户。

## 执行步骤

### Step 1: 创建虚拟显示器会话

使用 MCP 工具 `mcp__computer-use-offscreen__create_session`:
- width: 800
- height: 600

记录返回的 session_id。

### Step 2: 启动应用

使用 MCP 工具 `mcp__computer-use-offscreen__run_in_session`:
- session_id: 上一步返回的值
- command: `/root/DailyReport/build/DailyReport`

### Step 3: 等待主窗口

使用 MCP 工具 `mcp__computer-use-offscreen__wait_for_window`:
- title: "工时记录"
- timeout_ms: 10000

如果超时，截图保存错误状态并报告失败。

### Step 4: 截图主窗口

使用 MCP 工具 `mcp__computer-use-offscreen__computer`:
- action: "get_screenshot"

分析截图，确定"登录"按钮的坐标位置。

### Step 5: 点击登录按钮

使用 MCP 工具 `mcp__computer-use-offscreen__computer`:
- action: "left_click"
- coordinate: [从截图分析得到的坐标]

### Step 6: 等待登录对话框

使用 MCP 工具 `mcp__computer-use-offscreen__wait_for_window`:
- title: "登录"
- timeout_ms: 10000

### Step 7: 截图登录对话框

使用 MCP 工具 `mcp__computer-use-offscreen__computer`:
- action: "get_screenshot"

分析截图，确定：
- 用户名输入框坐标
- 密码输入框坐标
- 登录按钮坐标

### Step 8: 输入用户名

1. 点击用户名输入框: `action: "left_click", coordinate: [坐标]`
2. 输入用户名: `action: "type", text: "用户名"`
3. 截图

### Step 9: 输入密码

1. 点击密码输入框: `action: "left_click", coordinate: [坐标]`
2. 输入密码: `action: "type", text: "密码"`
3. 截图

### Step 10: 点击登录

1. 点击登录按钮: `action: "left_click", coordinate: [坐标]`
2. 等待 1 秒
3. 截图

### Step 11: 清理

使用 MCP 工具 `mcp__computer-use-offscreen__destroy_session`:
- session_id: 第一步返回的值

## 截图命名规范

截图按步骤编号命名：
- `01_main_window.png`
- `02_click_login_button.png`
- `03_login_dialog.png`
- `04_username_filled.png`
- `05_password_filled.png`
- `06_login_clicked.png`

## 错误处理

如果任何步骤超时或失败：
1. 截图保存当前状态（命名加 `_error` 后缀）
2. 记录错误信息
3. 继续执行清理步骤
4. 报告测试失败

## 完成后汇报

汇报以下内容：
- 每个步骤的执行结果
- 截图保存路径列表
- 发现的任何问题或异常
- 测试通过/失败结论
```

- [ ] **Step 2: 提交**

```bash
git add tests/ui/ui-test-prompt.md
git commit -m "test: add UI test prompt document for sub-agent execution"
```

---

### Task 3: 推送并验证

- [ ] **Step 1: 推送到远程**

```bash
git push origin master
```

- [ ] **Step 2: 验证文件结构**

```bash
ls -la /root/DailyReport/tests/ui/
```

期望输出：
```
README.md
ui-test-prompt.md
screenshots/
```

---

## 验收清单

- [ ] `/root/DailyReport/tests/ui/` 目录创建成功
- [ ] `README.md` 测试说明清晰
- [ ] `ui-test-prompt.md` 提示词完整，包含所有测试步骤
- [ ] 提示词包含错误处理和截图命名规范
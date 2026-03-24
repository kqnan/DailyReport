# DailyReport UI 测试提示词

你正在执行 DailyReport 应用的 UI 自动化测试。

## 测试目标

测试登录流程：主界面 -> 点击登录 -> 输入账号密码 -> 登录

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
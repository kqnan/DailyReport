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
2. 输入用户名: `action: "type", text: "<TEST_USERNAME环境变量的值>"`
3. 截图并保存

### Step 9: 输入密码

1. 点击密码输入框: `action: "left_click", coordinate: [坐标]`
2. 输入密码: `action: "type", text: "<TEST_PASSWORD环境变量的值>"`
3. 截图并保存

### Step 10: 点击登录

1. 点击登录按钮: `action: "left_click", coordinate: [坐标]`
2. 等待 1 秒（注：可直接继续下一步骤，MCP工具操作之间自然存在延迟；若需精确等待，可使用 `mcp__computer-use-offscreen__wait_for_window` 确认窗口状态）
3. 截图并保存

### Step 11: 清理

使用 MCP 工具 `mcp__computer-use-offscreen__destroy_session`:
- session_id: 第一步返回的值

## 截图保存说明

使用 MCP 工具 `mcp__computer-use-offscreen__computer` 的 `get_screenshot` action 捕获屏幕后，截图数据会返回给调用方。执行测试的子代理需要将截图保存到指定目录：
- 截图目录路径由配置参数中的"截图目录"指定
- 使用时间戳创建子目录以区分不同测试运行
- 按下方"截图命名规范"保存文件

## 截图命名规范

截图按步骤编号命名：
- `01_main_window.png`
- `02_click_login_button.png`
- `03_login_dialog.png`
- `04_username_filled.png`
- `05_password_filled.png`
- `06_login_clicked.png`

## 成功判定标准

测试判定为"通过"需满足以下条件：
1. 所有步骤均成功执行，无超时或异常
2. 登录对话框在点击登录按钮后成功关闭
3. 主界面显示登录成功状态（如用户信息、欢迎消息等）
4. 未出现任何错误提示框或错误消息

如果登录对话框未关闭或出现错误提示，则判定为"失败"。

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
# MCP UI 测试设计

## 概述

使用 MCP `computer-use-offscreen` 工具进行 Qt 桌面应用的 UI 自动化测试。

## 目标

创建 Python 测试脚本，自动化测试登录流程：
- 进入主界面 → 点击登录 → 输入账号密码 → 登录

## 文件结构

```
/root/DailyReport/tests/ui/
├── run_ui_test.py          # 主测试脚本
├── screenshots/             # 截图保存目录（自动创建）
│   └── 2026-03-24_123456/  # 按时间戳组织
│       ├── 01_main_window.png
│       ├── 02_click_login_button.png
│       ├── 03_login_dialog.png
│       ├── 04_username_filled.png
│       ├── 05_password_filled.png
│       └── 06_login_clicked.png
└── README.md                # 测试说明文档
```

## 测试流程

```
1. 创建虚拟显示器会话 (create_session)
2. 在会话中启动 DailyReport 应用 (run_in_session)
3. 等待主窗口出现 (wait_for_window, timeout=10s)
   - 成功 → 截图保存
   - 超时 → 报错退出
4. 点击"登录"按钮 (find_windows + computer.left_click)
   - 成功 → 截图保存
   - 超时 → 报错退出
5. 等待登录对话框出现 (wait_for_window, timeout=10s)
   - 成功 → 截图保存
   - 超时 → 报错退出
6. 输入账号 (computer.type)
   - 成功 → 截图保存
7. 输入密码 (computer.type)
   - 成功 → 截图保存
8. 点击登录按钮 (computer.left_click)
   - 成功 → 截图保存
9. 清理：销毁会话 (destroy_session)
```

## 核心 API

使用的 MCP 工具：

| 工具 | 用途 |
|------|------|
| `mcp__computer-use-offscreen__create_session` | 创建虚拟显示器 |
| `mcp__computer-use-offscreen__run_in_session` | 启动应用 |
| `mcp__computer-use-offscreen__wait_for_window` | 等待窗口出现 |
| `mcp__computer-use-offscreen__find_windows` | 查找窗口位置 |
| `mcp__computer-use-offscreen__computer` | 鼠标/键盘操作和截图 |
| `mcp__computer-use-offscreen__destroy_session` | 销毁会话 |

## 截图命名规范

格式：`{序号}_{操作描述}.png`

示例：
- `01_main_window.png`
- `02_click_login_button.png`
- `03_login_dialog.png`

## 错误处理

每个步骤设置超时时间，超时后：
1. 打印错误信息（哪一步超时）
2. 保存当前截图（便于调试）
3. 退出测试脚本（非零退出码）

## 运行方式

```bash
cd /root/DailyReport/tests/ui
python run_ui_test.py
```

## 配置参数

测试脚本顶部的可配置项：

```python
# 应用路径
APP_PATH = "/root/DailyReport/build/DailyReport"

# 默认超时时间（秒）
DEFAULT_TIMEOUT = 10

# 截图保存目录
SCREENSHOTS_DIR = "screenshots"

# 测试账号（可从环境变量读取）
USERNAME = os.environ.get("TEST_USERNAME", "test_user")
PASSWORD = os.environ.get("TEST_PASSWORD", "test_pass")
```

## 验收标准

1. 测试脚本能成功启动应用并完成登录流程
2. 每一步操作都有截图保存
3. 超时时能正确报错并保存调试截图
4. 测试完成后能正确清理资源
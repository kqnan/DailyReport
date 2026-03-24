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
# 工时记录应用 - 功能验证与完善计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 验证现有工时记录应用的功能完整性，确保所有需求都已正确实现

**Architecture:** 该应用使用 Tauri 2.x + Rust 后端 + React 前端。数据存储在用户数据目录的 JSON 文件中（按月组织）。需要验证：
1. 后端命令是否完整实现
2. 前端 UI 交互是否流畅
3. 数据持久化是否正确
4. 边界条件处理（如多段上班、窗口关闭时自动保存）

**Tech Stack:**
- Rust + Tauri 2.2 (backend)
- React 18 + TypeScript (frontend)
- Serde (serialization)
- UUID (session IDs)
- chrono (time handling)

**Existing Implementation Status:**
- ✅ `startShift()` - 开始上班命令
- ✅ `endShift(activity, work_type)` - 结束上班命令
- ✅ `getSessions(date)` - 查询指定日期记录
- ✅ `updateSession()` - 编辑记录
- ✅ `deleteSession()` - 删除记录
- ✅ `exportData(date, format)` - 导出 JSON/CSV
- ✅ `getTodayStatistics()` - 今日统计
- ✅ `getAvailableDates()` - 可用日期列表
- ✅ 窗口关闭时自动保存进行中的记录
- ✅ 系统托盘退出时自动保存

---

## 需求匹配矩阵

| 需求 | 实现状态 | 说明 |
|------|---------|------|
| 打开后 UI 显示"上班"按钮 | ✅ | `App.tsx:219` |
| 点击后开始上班 | ✅ | `handleStartShift()`, `start_shift()` |
| 再次点击下班 | ✅ | `handleEndShift()`, `end_shift()` |
| 上班后软件开始计时 | ✅ | `time_tracker.rs:4-10` |
| 下班或关闭时终止计时并保存 | ✅ | `main.rs:54-64`, `storage.rs:150-161` |
| 保存时长（小时，1位小数） | ✅ | `models.rs:28-42` |
| 允许多段上班时间 | ✅ | `get_today_sessions()` 支持多记录 |
| 查询过往日期记录 | ✅ | `get_sessions()`, 日期选择器 |
| 编辑每段的工作内容（默认文本） | ✅ | `update_session_cmd()`, 默认"工作片段" |

---

## 潜在问题与改进建议

### 高优先级

### Task 1: 验证数据持久化路径

**文件:** `src-tauri/src/storage.rs:13-19`

- [ ] **步骤 1:** 运行应用并验证数据目录创建

```bash
# 构建并运行应用
cd src-tauri && cargo run
```

- [ ] **步骤 2:** 检查数据目录是否存在（Linux 示例）

```bash
ls -la ~/.local/share/com/daily-report/DailyReport/work_sessions/
```

预期：至少有一个 `sessions_2026_03.json` 文件

- [ ] **步骤 3:** 验证 JSON 文件格式

```bash
# 查看文件内容（应为有效的 JSON）
cat ~/.local/share/com/daily-report/DailyReport/work_sessions/sessions_*.json
```

### Task 2: 验证时间舍入逻辑

**文件:** `src-tauri/src/models.rs:28-42`

- [ ] **步骤 1:** 添加单元测试验证舍入行为

```rust
// tests/test_models.rs
#[test]
fn test_duration_rounding() {
    // 1小时30分钟 = 1.5小时
    // 1小时35分钟 ≈ 1.6小时（35/60*10=5.83，round=6，6/10=0.6）
    // 验证精确到小数点后一位
}
```

- [ ] **步骤 2:** 运行测试

```bash
cd src-tauri && cargo test test_duration_rounding
```

### Task 3: 验证前端默认文本逻辑

**文件:** `frontend/src/App.tsx:269`

- [ ] **步骤 1:** 检查默认文本显示逻辑

当前代码：`session.activity !== '工作片段' ? session.activity : '无描述'`

问题：如果用户输入"工作片段"作为 activity，会显示"无描述"而不是原始文本

- [ ] **步骤 2:** 修复建议（可选）

可以改为：
```typescript
{session.activity || '无描述'}
```
或在存储时使用空字符串作为默认值

### 中优先级

### Task 4: 验证多段上班切换逻辑

**文件:** `src-tauri/src/commands.rs:7-21`

- [ ] **步骤 1:** 测试在已有进行中记录时再次点击"上班"

预期行为：应显示错误提示"已经有进行中的上班记录，请先下班"

- [ ] **步骤 2:** 在 `App.tsx:94-96` 验证错误显示

```bash
# 构建前端验证编译
cd frontend && npm run build
```

### Task 5: 验证窗口关闭自动保存

**文件:** `src-tauri/src/main.rs:54-64`

- [ ] **步骤 1:** 启动应用并开始一个上班记录
- [ ] **步骤 2:** 直接关闭窗口（不点击下班）
- [ ] **步骤 3:** 重新打开应用，检查记录是否已自动结束

### 低优先级（功能增强）

### Task 6: 添加运行中时长实时刷新

**当前:** 上班后时长不更新，只有下班后才有准确时长

- [ ] **步骤 1:** 添加定时器刷新运行中会话的显示时长
- [ ] **步骤 2:** 更新前端 `useEffect` 添加实时计算

## 验证测试命令

### 编译验证

```bash
# 验证 Rust 代码
cd src-tauri && cargo check

# 验证前端代码
cd frontend && npm run build
```

### 功能测试检查表

- [ ] **FT-01:** 启动应用，点击"上班"按钮，按钮变为"下班"，显示开始时间
- [ ] **FT-02:** 点击"下班"按钮，弹出对话框，输入工作内容后确认
- [ ] **FT-03:** 检查统计卡片显示正确时长（格式：X小时或X分钟）
- [ ] **FT-04:** 选择其他日期，验证能查看历史记录
- [ ] **FT-05:** 编辑某条记录，更改工作内容，验证保存成功
- [ ] **FT-06:** 删除某条记录，验证可以从列表中移除
- [ ] **FT-07:** 开始一个记录后直接关闭窗口，重新打开验证记录已自动结束
- [ ] **FT-08:** 导出 CSV/JSON 文件，验证格式正确
- [ ] **FT-09:** 尝试在已有进行中记录时再次点击"上班"，验证显示错误提示
- [ ] **FT-10:** 一天内创建多段记录，验证都能正确保存和显示

---

## 单元测试结果

已创建完整的单元测试，所有测试通过：

```
running 12 tests
test test_end_shift_sets_end_time ... ok
test test_format_duration_minutes ... ok
test test_format_duration_mixed ... ok
test test_format_duration_zero ... ok
test test_format_duration_hours ... ok
test test_start_shift_creates_session ... ok
test test_work_session_duration_calculation ... ok
test test_work_session_creation ... ok
test test_work_session_duration_rounding ... ok
test test_work_session_exact_hour ... ok
test test_work_session_is_active ... ok
test test_work_session_minutes_only ... ok

test result: ok. 12 passed; 0 failed; 0 ignored
```

**测试覆盖:**
- `WorkSession::new()` - 会话创建
- `WorkSession::calculate_duration()` - 时长计算（精确到小数点后一位）
- `WorkSession::is_active()` - 激活状态检查
- `start_shift()` - 开始上班
- `end_shift()` - 结束上班
- `format_duration()` - 时长格式化（小时、分钟、混合）

**代码质量:**
- `cargo check` - 通过
- `cargo clippy` - 无警告
- `cargo build` - 成功

---

## 执行指导

Plan complete and saved to `docs/superpowers/plans/2026-03-11-work-hours-app-verification.md`.

To execute the manual testing checklist, run:
```bash
cd src-tauri && cargo run
```

Then follow the functional test checklist (FT-01 through FT-10) in the document.

---
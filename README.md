# Qt 工时记录应用

使用 Qt 6.6 开发的工时记录桌面应用。

## 功能特点

- 上班/下班记录
- 时长计算（精确到小数点后一位）
- 多段记录支持
- 历史记录查询
- 记录编辑和删除
- 导出 JSON/CSV
- 关闭时自动保存进行中的记录

## 编译要求

- Qt 6.6+
- CMake 3.28+
- C++20 支持的编译器

## 编译步骤

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## 运行应用

```bash
cd build
./DailyReport
```

## 打包

### Linux

```bash
# 使用 appimagetool 打包 AppImage
appimagetool package/linux/DailyReport/
```

### Windows

```powershell
# 使用 windeployqt
& "C:\Qt\6.6\...\bin\windeployqt.exe" --dir package/windows/DailyReport package/windows/DailyReport/DailyReport.exe
```

### macOS

```bash
# 创建 app bundle
# 使用 codesign 签名（可选）
```

## 数据存储

数据存储在用户数据目录的 `work_sessions/` 文件夹中，按月组织为 JSON 文件：
- `sessions_2026_03.json`
- `sessions_2026_04.json`
- 等等

# macOS 打包说明

## 手动打包步骤

1. 构建应用
```bash
cmake -S . -B build
cmake --build build
```

2. 创建 app bundle 结构
```bash
mkdir -p DailyReport.app/Contents/MacOS
mkdir -p DailyReport.app/Contents/Frameworks
mkdir -p DailyReport.app/Contents/Plugins
```

3. 复制二进制文件
```bash
cp -f build/DailyReport DailyReport.app/Contents/MacOS/
```

4. 复制 Qt 框架
```bash
# 使用 macdeployqt 工具
macdeployqt DailyReport.app -verbose=2
```

5. 签名（可选）
```bash
codesign --deep --force --verbose --sign "Developer ID Application: Your Name" DailyReport.app
```

6. 打包为 dmg（可选）
```bash
hdiutil create -volname "DailyReport" -srcfolder DailyReport.app DailyReport.dmg
```

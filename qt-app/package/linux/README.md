# Linux 打包说明

## 构建和打包步骤

1. 构建应用
```bash
cd /root/DailyReport/qt-app
mkdir -p build && cd build
cmake ..
cmake --build .
```

2. 运行部署脚本
```bash
cd /root/DailyReport/qt-app
./package/linux/deploy.sh
```

3. 创建 AppImage（可选）
```bash
# 安装 appimagetool
wget -O appimagetool https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
chmod +x appimagetool

# 创建 AppImage
./appimagetool package/linux/DailyReport DailyReport.AppImage
```

## 部署结构

生成的应用目录结构：
```
DailyReport/
├── DailyReport           # 可执行文件
├── AppRun               # AppImage 运行链接
├── DailyReport.desktop  # 桌面文件
└── plugins/
    └── platforms/
        └── libqxcb.so   # Qt 平台插件
```

## 运行应用

直接运行：
```bash
./package/linux/DailyReport/DailyReport
```

或运行 AppImage：
```bash
./DailyReport.AppImage
```

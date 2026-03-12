#!/bin/bash

APP_NAME="DailyReport"
BUILD_DIR="build"
APP_DIR="package/linux/${APP_NAME}"

rm -rf "${APP_DIR}"
mkdir -p "${APP_DIR}/plugins/platforms"
mkdir -p "${APP_DIR}/plugins/styles"

# Copy binary from build directory
cp -f "${BUILD_DIR}/${APP_NAME}" "${APP_DIR}/"

# Copy Qt plugins from built plugins directory
if [ -d "${BUILD_DIR}/plugins/platforms" ]; then
    cp -f "${BUILD_DIR}/plugins/platforms/libqxcb.so" "${APP_DIR}/plugins/platforms/"
else
    # Fallback: find Qt plugins using qtpaths6
    QT_PLUGINS_DIR=$(qtpaths6 --paths GenericDataLocation 2>/dev/null | cut -d: -f1)
    if [ -n "${QT_PLUGINS_DIR}" ]; then
        QT_PLUGINS_DIR="${QT_PLUGINS_DIR}/../plugins"
    fi
    # Try common locations
    for loc in /usr/lib64/qt6/plugins /usr/lib/qt6/plugins /usr/share/qt6/plugins; do
        if [ -d "${loc}/platforms" ]; then
            QT_PLUGINS_DIR="${loc}"
            break
        fi
    done

    if [ -d "${QT_PLUGINS_DIR}/platforms" ]; then
        cp -f "${QT_PLUGINS_DIR}/platforms/libqxcb.so" "${APP_DIR}/plugins/platforms/"
        echo "Copied Qt plugins from: ${QT_PLUGINS_DIR}"
    else
        echo "Warning: Could not find Qt plugins directory"
    fi
fi

# Create desktop file
cat > "${APP_DIR}/${APP_NAME}.desktop" << EOF
[Desktop Entry]
Name=${APP_NAME}
Exec=${APP_NAME}
Type=Application
Categories=Utility;
EOF

# Create AppRun symlink
cd "${APP_DIR}"
ln -sf ${APP_NAME} AppRun
cd - > /dev/null

echo "App directory created at: ${APP_DIR}"

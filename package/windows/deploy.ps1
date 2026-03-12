$AppDir = "package/windows/DailyReport"
New-Item -ItemType Directory -Path $AppDir -Force

Copy-Item "build\Release\DailyReport.exe" -Destination $AppDir

# Find Qt installation using environment variables
$QtDir = $env:QT_ROOT_DIR
if ([string]::IsNullOrEmpty($QtDir)) {
    $QtDir = "C:\Qt\6.6\msvc2019_64"
}

Write-Host "Qt installation directory: $QtDir"
Write-Host "App directory: $AppDir"

if (Test-Path "$QtDir\bin\windeployqt.exe") {
    & "$QtDir\bin\windeployqt.exe" --dir $AppDir $AppDir\DailyReport.exe

    # Copy platform plugin explicitly
    Copy-Item "$QtDir\plugins\platforms\qwindows.dll" -Destination "$AppDir\platforms\" -Force
} else {
    Write-Error "windeployqt.exe not found at $QtDir\bin"
    exit 1
}

Write-Host "Package created at: $AppDir"

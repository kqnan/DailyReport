$AppDir = "package/windows/DailyReport"
New-Item -ItemType Directory -Path $AppDir -Force

Copy-Item "build\DailyReport.exe" -Destination $AppDir

# Find Qt installation
$QtDir = "C:\Qt\6.6\msvc2019_64"
if (Test-Path $QtDir) {
    & "$QtDir\bin\windeployqt.exe" --dir $AppDir $AppDir\DailyReport.exe
} else {
    Write-Warning "Qt not found at $QtDir. Please install Qt 6.6 and run again."
}

Write-Host "Package created at: $AppDir"

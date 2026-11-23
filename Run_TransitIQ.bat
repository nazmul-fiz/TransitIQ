@echo off
title TransitIQ - Smart Bus Network Explorer (Qt6 GUI)
chcp 65001 >nul
cd /d "%~dp0"

set "PATH=C:\Qt\6.6.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Program Files\CMake\bin;%PATH%"

if not exist "TransitIQ.exe" (
    echo [INFO] TransitIQ.exe not found. Building with CMake and Qt6...
    call build.bat
    exit /b 0
)

start "" "TransitIQ.exe"

@echo off
title TransitIQ - Smart Bus Network Explorer
chcp 65001 >nul
cd /d "%~dp0"

if not exist "TransitIQ.exe" (
    echo [INFO] TransitIQ.exe not found. Building GUI executable...
    g++ -std=c++17 -O2 -Wall -Wextra -Iinclude src/Common.cpp src/Graph.cpp src/BusManager.cpp src/DataManager.cpp src/UI.cpp src/App.cpp src/GUIApp.cpp src/main.cpp -mwindows -lcomctl32 -lgdi32 -o TransitIQ.exe
    if %errorlevel% neq 0 (
        echo [ERROR] Compilation failed.
        pause
        exit /b 1
    )
)

start "" "TransitIQ.exe"

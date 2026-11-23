@echo off
title TransitIQ - Smart Bus Network Explorer
chcp 65001 >nul
cd /d "%~dp0"

if not exist "TransitIQ.exe" (
    echo [INFO] TransitIQ.exe not found. Building executable...
    g++ -std=c++17 -O2 -Wall -Wextra -Iinclude src/Common.cpp src/Graph.cpp src/BusManager.cpp src/DataManager.cpp src/UI.cpp src/App.cpp src/main.cpp -o TransitIQ.exe
    if %errorlevel% neq 0 (
        echo [ERROR] Compilation failed.
        pause
        exit /b 1
    )
)

cls
TransitIQ.exe

if %errorlevel% neq 0 (
    echo.
    echo Application exited with error code %errorlevel%.
    pause
)

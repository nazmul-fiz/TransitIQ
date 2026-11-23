@echo off
echo ===================================================
echo   TransitIQ - GUI & Engine Build Script
echo ===================================================
echo.

where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] MinGW g++ compiler was not found in your PATH!
    echo Please install MinGW or add its bin directory to PATH.
    pause
    exit /b 1
)

echo [1/2] Compiling TransitIQ GUI application (C++17)...
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude src/Common.cpp src/Graph.cpp src/BusManager.cpp src/DataManager.cpp src/UI.cpp src/App.cpp src/GUIApp.cpp src/main.cpp -mwindows -lcomctl32 -lgdi32 -o TransitIQ.exe

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Compilation failed! Please check error messages above.
    pause
    exit /b 1
)

echo [2/2] Build successful: TransitIQ.exe
echo.
echo Launching TransitIQ Visual GUI...
echo ===================================================
start TransitIQ.exe

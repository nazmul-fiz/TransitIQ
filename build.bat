@echo off
echo ===================================================
echo   TransitIQ - Qt6 & CMake One-Click Build Script
echo ===================================================
echo.

set "PATH=C:\Qt\6.6.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Program Files\CMake\bin;%PATH%"

if not exist build mkdir build
cd build

echo [1/3] Configuring CMake with Qt6...
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.6.3/mingw_64" ..
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed!
    pause
    exit /b 1
)

echo [2/3] Building TransitIQ executable...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo [3/3] Deploying Qt6 runtime libraries...
copy /y TransitIQ.exe ..\TransitIQ.exe >nul
copy /y test_algorithms.exe ..\test_algorithms.exe >nul
cd ..
windeployqt.exe --no-translations --no-compiler-runtime TransitIQ.exe >nul

echo.
echo ===================================================
echo   Build Successful! Launching TransitIQ Qt6 GUI...
echo ===================================================
start "" "TransitIQ.exe"

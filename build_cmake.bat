@echo off
rem ==========================================================================
rem  MAZE-X  -  build everything with CMake (works with any MinGW or Visual Studio)
rem  Needs: CMake 3.24+ and Git on PATH. If SFML 3 is not installed, CMake
rem  downloads SFML 3.0.2 and builds it with your own compiler (first time: a few minutes).
rem ==========================================================================
setlocal
where cmake >nul 2>nul
if errorlevel 1 (
    echo CMake not found. Install it from https://cmake.org/download/ and tick "Add to PATH".
    exit /b 1
)

set GENERATOR=
where g++ >nul 2>nul
if not errorlevel 1 set GENERATOR=-G "MinGW Makefiles"

cmake -S . -B build %GENERATOR% -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
cmake --build build --config Release -j 4
if errorlevel 1 exit /b 1
ctest --test-dir build -C Release --output-on-failure
if errorlevel 1 exit /b 1

echo.
echo Done. The programs are in build\  (build\Release\ with Visual Studio):
echo    MazeX.exe   mazex_console.exe   mazex_tests.exe
endlocal

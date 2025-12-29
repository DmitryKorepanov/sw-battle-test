@echo off
setlocal

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do set VS_PATH=%%i

if not defined VS_PATH (
    echo Error: Visual Studio not found
    exit /b 1
)

call "%VS_PATH%\Common7\Tools\VsDevCmd.bat" -arch=amd64

if not exist build-clang mkdir build-clang
cd build-clang

cmake -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl ..
if errorlevel 1 goto :error

cmake --build . --config Release
if errorlevel 1 goto :error

echo.
echo === Build successful! ===
goto :end

:error
echo.
echo === Build failed! ===
exit /b 1

:end


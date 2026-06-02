@echo off

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do set VS_PATH=%%i

if "%VS_PATH%"=="" (
    echo Visual Studio not found!
    pause
    exit /b
)

CALL "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"

powershell

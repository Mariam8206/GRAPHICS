@echo off
rem Builds the game (x64 Debug) with the Visual Studio 2026 Build Tools, then runs it.
rem Usage: double-click this file, or type  .\build_and_run.bat  in a terminal inside this folder.

set MSBUILD="C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"

%MSBUILD% "%~dp0Template.vcxproj" -p:Configuration=Debug -p:Platform=x64 -p:PlatformToolset=v145 -v:minimal -nologo
if errorlevel 1 (
    echo.
    echo BUILD FAILED - read the errors above.
    pause
    exit /b 1
)

echo Starting the game...
"%~dp0x64\Debug\Template.exe"

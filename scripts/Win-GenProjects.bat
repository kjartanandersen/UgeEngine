@echo off
REM Generates the Visual Studio solution. Works from any directory.
REM premake5.lua first copies in the dependency scripts from premake/thirdparty/ and applies
REM patches/, and stops with an error if a submodule's copy differs.
pushd "%~dp0.."
call thirdparty\premake\bin\premake5.exe vs2026
set "RESULT=%ERRORLEVEL%"
popd
exit /b %RESULT%

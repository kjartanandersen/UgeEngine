@echo off
REM Generates the Visual Studio solution. Works from any directory.
REM premake5.lua first restores the dependency build scripts and patches kept in
REM premake/thirdparty/ and patches/, and stops with an error if one has diverged.
pushd "%~dp0.."
call thirdparty\premake\bin\premake5.exe vs2026
set "RESULT=%ERRORLEVEL%"
popd
exit /b %RESULT%

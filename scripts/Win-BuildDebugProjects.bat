@echo off
REM Builds the whole solution in Debug. Works from any directory.
pushd "%~dp0.."
call msbuild Uge.slnx /p:Configuration=Debug /p:Platform=x64 /m
set "RESULT=%ERRORLEVEL%"
popd
exit /b %RESULT%
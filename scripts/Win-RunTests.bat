@echo off
REM Builds and runs Uge-Tests, the GoogleTest runner for the engine and the editor.
REM
REM Usage:
REM   scripts\Win-RunTests.bat [Debug, Release or Dist] [--no-build] [--filter PATTERN] [--brief] [GTEST_ARGS...]
REM
REM   Debug, Release, Dist   Configuration to build and run. Default: Debug.
REM   --no-build             Run the existing binary instead of building it first.
REM   --filter PATTERN       Run only matching tests, e.g. --filter SceneSerializerTest.*
REM   --brief                Print only failing tests (GoogleTest's --gtest_brief=1).
REM   GTEST_ARGS             Anything else is passed to the runner, e.g. --gtest_repeat=5.
REM                          A --gtest_ flag that cmd split at its '=' is rejoined.
REM
REM Works from any directory, and runs the tests from the repository root. Builds first
REM unless --no-build is given. Exits with the runner's exit code.

setlocal EnableExtensions

set "ROOT=%~dp0.."
set "CONFIG=Debug"
set "BUILD=1"
set "GTEST_ARGS="

:parse
if "%~1"=="" goto parsed
if /i "%~1"=="-h" goto usage
if /i "%~1"=="--help" goto usage
if "%~1"=="/?" goto usage
if /i "%~1"=="Debug" (
    set "CONFIG=Debug"
    shift
    goto parse
)
if /i "%~1"=="Release" (
    set "CONFIG=Release"
    shift
    goto parse
)
if /i "%~1"=="Dist" (
    set "CONFIG=Dist"
    shift
    goto parse
)
if /i "%~1"=="--no-build" (
    set "BUILD=0"
    shift
    goto parse
)
if /i "%~1"=="--brief" (
    set "GTEST_ARGS=%GTEST_ARGS% --gtest_brief=1"
    shift
    goto parse
)
if /i "%~1"=="--filter" goto parse_filter
goto parse_passthrough

REM Anything else goes to GoogleTest. cmd splits at '=' and PowerShell drops the quotes, so
REM --gtest_repeat=5 can arrive as two arguments. They are joined again, unless the second
REM starts with --, which means the first was a flag with no value.
:parse_passthrough
set "ARG=%~1"
set "NEXT=%~2"
if /i not "%ARG:~0,8%"=="--gtest_" goto parse_passthrough_as_is
if "%NEXT%"=="" goto parse_passthrough_as_is
if "%NEXT:~0,2%"=="--" goto parse_passthrough_as_is
echo(%ARG%| findstr /c:"=" >nul && goto parse_passthrough_as_is
set "GTEST_ARGS=%GTEST_ARGS% "%ARG%=%NEXT%""
shift
shift
goto parse

:parse_passthrough_as_is
set "GTEST_ARGS=%GTEST_ARGS% %1"
shift
goto parse

:parse_filter
if "%~2"=="" (
    echo [RunTests] ERROR: --filter needs a pattern, e.g. --filter SceneSerializerTest.*
    exit /b 2
)
set "GTEST_ARGS=%GTEST_ARGS% "--gtest_filter=%~2""
shift
shift
goto parse

:parsed
set "EXE=%ROOT%\bin\%CONFIG%-windows-x86_64\Uge-Tests\Uge-Tests.exe"

if "%BUILD%"=="0" goto run

call :find_msbuild
if defined MSBUILD goto build
echo [RunTests] ERROR: MSBuild was not found. Install Visual Studio with the C++ workload,
echo [RunTests]        run this from a Developer Command Prompt, or pass --no-build.
exit /b 1

:build
REM Builds the project, and with it Uge, Jolt and GoogleTest as needed.
echo [RunTests] Building Uge-Tests (%CONFIG%)...
"%MSBUILD%" "%ROOT%\Uge-Tests\Uge-Tests.vcxproj" /p:Configuration=%CONFIG% /p:Platform=x64 /m /nologo /v:minimal
if errorlevel 1 goto build_failed
goto run

:build_failed
echo [RunTests] ERROR: the build failed; not running the tests.
exit /b 1

:run
if exist "%EXE%" goto run_exe
echo [RunTests] ERROR: %EXE% does not exist. Build it first, or drop --no-build.
exit /b 1

:run_exe
pushd "%ROOT%"
echo [RunTests] Running bin\%CONFIG%-windows-x86_64\Uge-Tests\Uge-Tests.exe%GTEST_ARGS%
"%EXE%"%GTEST_ARGS%
set "RESULT=%ERRORLEVEL%"
popd
if not "%RESULT%"=="0" echo [RunTests] Tests FAILED (exit code %RESULT%).
exit /b %RESULT%

:usage
echo Builds and runs Uge-Tests from the repository root.
echo.
echo Usage: scripts\Win-RunTests.bat [Debug, Release or Dist] [--no-build] [--filter PATTERN] [--brief] [GTEST_ARGS...]
echo.
echo   Debug, Release, Dist   Configuration to build and run. Default: Debug.
echo   --no-build             Run the existing binary instead of building it first.
echo   --filter PATTERN       Run only matching tests, e.g. --filter SceneSerializerTest.*
echo   --brief                Print only failing tests.
echo   GTEST_ARGS             Passed to GoogleTest, e.g. --gtest_repeat=5
exit /b 0

REM Sets MSBUILD to the msbuild on PATH, or else the one vswhere finds. Empty if neither.
:find_msbuild
set "MSBUILD="
where msbuild >nul 2>nul
if errorlevel 1 goto find_msbuild_vswhere
set "MSBUILD=msbuild"
exit /b 0

:find_msbuild_vswhere
REM Kept out of any parenthesised block: the ')' in "Program Files (x86)" would end it.
set "VSWHERE_DIR=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"
if not exist "%VSWHERE_DIR%\vswhere.exe" exit /b 0
set "PATH=%VSWHERE_DIR%;%PATH%"
for /f "usebackq delims=" %%i in (`vswhere -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    if not defined MSBUILD set "MSBUILD=%%i"
)
exit /b 0
